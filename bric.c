#include "bric.h"

static struct editor_config Editor;
static struct termios orig_termios; // so we can restore original at exit


// Low level terminal handling
void disable_raw_mode(int fd)
{
        // dont bother checking the return value as its too late
        if (Editor.rawmode) {
                tcsetattr(fd, TCSAFLUSH, &orig_termios);
                write(fd, "\033[2J\033[H\033[?1049l", 15);
                Editor.rawmode = 0;
        }
}

void editor_at_exit(void)
{
        disable_raw_mode(STDIN_FILENO);
}

int enable_raw_mode(int fd)
{
        struct termios raw;

        if(Editor.rawmode) return 0; //already enabled
        if(!isatty(fd)) goto fatal;
        atexit(editor_at_exit);
        if(tcgetattr(fd, &orig_termios) == -1) goto fatal;

        raw = orig_termios; // modify the original mode
        /* input modes: no break, no CR to NL, no parity check, no strip char,
         *      * no start/stop output control. */
        raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

        // output modes - disable post processing
        raw.c_oflag &= ~(OPOST);

        //control modes - set 8 bit chars
        raw.c_cflag |= (CS8);

        //local modes, choing off, canonical off, no extended functions, no signal chars (, etc)
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

        //control chars - set return condition: min number of bytes and a timer
        raw.c_cc[VMIN] = 0; // return each byte, or zero for a timeout
        raw.c_cc[VTIME] = 1; //100ms timeout

        //put terminal in raw mode after flushing
        if(tcsetattr(fd, TCSAFLUSH, &raw) < 0) goto fatal;
        //if(write(fd, "\033[?1049h\033[2J\033[H", 15) != 15) goto fatal;
        Editor.rawmode = 1;
        return 0;

fatal:
        errno = ENOTTY;
        return -1;
}


// read a key from terminal input in raw mode and handle
int editor_read_key(int fd)
{
        int nread;
        char c, seq[3];
        while((nread = read(fd, &c, 1)) == 0);
        if(nread == -1) exit(1);

        while(1) {
                switch(c) {
                        case ESC: // escape sequence
                                // if its an escape then we timeout here
                                if(read(fd, seq, 1) == 0) return ESC;
                                if(read(fd, seq+1, 1) == 0) return ESC;

                                // ESC [ sequences
                                if(seq[0] == '[') {
                                        if(seq[1] >= '0' && seq[1] <= '9') {
                                                // extended escape so read additional byte
                                                if(read(fd, seq+2, 1) == 0) return ESC;
                                                if(seq[2] == '~') {
                                                        switch(seq[1]) {
                                                                case '3': return DEL_KEY;
                                                                case '5': return PAGE_UP;
                                                                case '6': return PAGE_DOWN;
                                                        }
                                                }
                                        } else {
                                                switch(seq[1]) {
                                                        case 'A': return ARROW_UP;
                                                        case 'B': return ARROW_DOWN;
                                                        case 'C': return ARROW_RIGHT;
                                                        case 'D': return ARROW_LEFT;
                                                        case 'H': return HOME_KEY;
                                                        case 'F': return END_KEY;
                                                }
                                        }
                                }

                                // ESC 0 sequences
                                else if(seq[0] == 'O') {
                                        switch (seq[1]) {
                                                case 'H': return HOME_KEY;
                                                case 'F': return END_KEY;
                                        }
                                }
                                break;

                        default:
                                return c;
                }
        }
}

/* Use the ESC [6n escape sequence to query the horizontal cursor position
 *  * and return it. On error -1 is returned, on success the position of the
 *   * cursor is stored at *rows and *cols and 0 is returned. */
int get_cursor_pos(int ifd, int ofd, int *rows, int *columns)
{
        char buf[32];
        unsigned int i = 0;

        // report cursor location
        if (write(ofd, "\x1b[6n", 4) != 4) return -1;

        // read the response: ESC [ rows; columns R
        while(i < sizeof(buf)-1) {
            if( read(ifd, buf+i, 1) != 1) break;
            if(buf[i] == 'R') break;
            i++;
        }
        buf[i] = '\0';

        // parse it
        if (buf[0] != ESC || buf[1] != '[') return -1;
        if (sscanf(buf+2,"%d;%d",rows,columns) != 2) return -1;
        return 0;

}


// try to get the number of columns in the window
int get_window_size(int ifd, int ofd, int *rows, int *columns)
{
        struct winsize ws;

        if(ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
                // ioctl() failed so query the terminal itself
                int original_row, original_column, retval;

                // get the initail position to store for later
                retval = get_cursor_pos(ifd, ofd, &original_row, &original_column);
                if(retval == -1) goto failed;

                // go to the right bottom margin and get position
                if(write(ofd, "\x1b[999C\x1b[999B", 12) != 12) goto failed;
                retval = get_cursor_pos(ifd, ofd, rows, columns);
                if(retval == -1) goto failed;

                // restore the cursor position
                char seq[32];
                snprintf(seq, 32, "\x1b[%d;%dH", original_row, original_column);
                if(write(ofd, seq, strlen(seq)) == -1) {
                        // cant recover the cursor pos ....
                }
                return 0;
        } else {
                *columns = ws.ws_col;
                *rows = ws.ws_row;
                return 0;
        }
failed:
        return -1;
}


/// SYNTAX HIGHLIGHTING!!

int is_separator(int c)
{
        return c == '\0' || isspace(c) || strchr(",.()+_/*=%[];", c) != NULL;
}


// return true if the specified row's last char is part of a multiline comment that spans to the next row
int editing_row_has_open_comment(editing_row *row)
{
        if(row->hl && row->rendered_size && row->hl[row->rendered_size-1] == HL_MLCOMMENT && (row->rendered_size < 2 || (row->rendered_chars[row->rendered_size-2] != '*' || row->rendered_chars[row->rendered_size-1] != '/'))) return 1;

        return 0;
}


// set every byte of row->hl (every character in the line) to the right syntax type defined by HL_*
void editor_update_syntax(editing_row *row)
{
        row->hl = realloc(row->hl, row->rendered_size);
        memset(row->hl, HL_NORMAL, row->rendered_size);

        if(Editor.syntax == NULL) return; //no syntax in this line, everything is HL_NORMAL

        int i, prev_sep, in_string, in_comment;
        char *p;
        char **keywords = Editor.syntax->keywords;
        char *scs = Editor.syntax->singleline_comment_start;
        char *mcs = Editor.syntax->multiline_comment_start;
        char *mce = Editor.syntax->multiline_comment_end;

        // point to the first non space char
        p = row->rendered_chars;
        i = 0; // current char offset in row
        while (*p && isspace(*p)) {
                p++;
                i++;
        }
        prev_sep = 1; // tell the parser if 'i' points to the start of a word
        in_string = 0; // is the character  in a string "" or ''
        in_comment = 0; // is the char in an open multiline comment

        //if the previous line has open open comment then this line start with an open comment state
        if(row->index > 0 && editing_row_has_open_comment(&Editor.row[row->index-1])) in_comment = 1;

        while (*p) {
                 //handle single line comments
                if(prev_sep && *p == scs[0] && *(p+1) == scs[1]) {
                        // from here to the end of the row is a comment
                        memset(row->hl+i, HL_COMMENT, row->rendered_size-i);
                        return;
                }

                // handle multiline comments
                if(in_comment) {
                        row->hl[i] = HL_MLCOMMENT;
                        if(*p == mce[0] && *(p+1) == mce[1]) {
                                row->hl[i+1] = HL_MLCOMMENT;
                                p += 2; i+= 2;
                                in_comment = 0;
                                prev_sep = 1;
                                continue;
                        } else {
                                prev_sep = 0;
                                p++; i++;
                                continue;
                        }
                } else if(*p == mcs[0] && *(p+1) == mcs[1]) {
                        row->hl[i] = HL_MLCOMMENT;
                        row->hl[i+1] = HL_MLCOMMENT;
                        p += 2; i += 2;
                        in_comment = 1;
                        prev_sep = 0;
                        continue;
                }


                // handle "" and ''
                if(in_string) {
                        row->hl[i] = HL_STRING;
                        if(*p == '\\') {
                                row->hl[i+1] = HL_STRING;
                                p += 2; i += 2;
                                prev_sep = 0;
                                continue;
                        }
                        if(*p == in_string) in_string = 0;
                        p++; i++;
                        continue;
                } else {
                        if(*p == '"' || *p == '\'') {
                                in_string = *p;
                                row->hl[i] = HL_STRING;
                                p++; i++;
                                prev_sep = 0;
                                continue;
                        }
                }

                // handle non printable chars
                if(!isprint(*p)) {
                        row->hl[i] = HL_NONPRINT;
                        p++; i++;
                        prev_sep = 0;
                        continue;
                }

                //handle numbers
                if((isdigit(*p) && (prev_sep || row->hl[i-1] == HL_NUMBER)) || (*p == '.' && i > 0 && row->hl[i-1] == HL_NUMBER)) {
                        row->hl[i] = HL_NUMBER;
                        p++; i++;
                        prev_sep = 0;
                        continue;
                }

                //handle keywords and lib calls
                if(prev_sep) {
                        int j;
                        for(j = 0; keywords[j]; j++) {
                                int klen = strlen(keywords[j]);
                                int pp = keywords[j][klen-1] == '|'; // preprocessor keyword
				int cond = keywords[j][klen-1] == '~'; // condition
				int retu = keywords[j][klen-1] == '#';
				int adapter = keywords[j][klen-1] == '^'; //adapter keywords
    				int loopy = keywords[j][klen-1] == '@';
				if(pp || cond || retu || adapter || loopy) klen--;

                                if(!memcmp(p, keywords[j], klen) && is_separator(*(p+klen))) {
                                        // keyword
                                        if(pp) memset(row->hl+i, HL_KEYWORD_PP, klen);
					else if(cond) memset(row->hl+i, HL_KEYWORD_COND, klen);
                                        else if(retu) memset(row->hl+i, HL_KEYWORD_RETURN, klen);
                                        else if(adapter) memset(row->hl+i, HL_KEYWORD_ADAPTER, klen);
                                        else if(loopy) memset(row->hl+i, HL_KEYWORD_LOOP, klen);
                                        else memset(row->hl+i, HL_KEYWORD_TYPE, klen);
                                        p += klen;
                                        i += klen;
                                        break;
                                }
                        }
                        if(keywords[j] != NULL) {
                                prev_sep = 0;
                                continue; // we had a keyword match
                        }
                }

                // not special chars
                prev_sep = is_separator(*p);
                p++; i++;
        }

        // propagate a syntax change to the next row if the open comment state is changed
        int oc = editing_row_has_open_comment(row);
        if(row->hl_open_comment != oc && row->index+1 < Editor.num_of_rows) editor_update_syntax(&Editor.row[row->index+1]);
        row->hl_open_comment = oc;
}

// map thee syntax highlight types to terminal colours
int editor_syntax_to_colour(int highlight)
{
        switch(highlight) {
                case HL_COMMENT: return Editor.colours.hl_comment_colour;
                case HL_MLCOMMENT: return Editor.colours.hl_mlcomment_colour;
                case HL_KEYWORD_COND: return Editor.colours.hl_keyword_cond_colour;
                case HL_KEYWORD_TYPE: return Editor.colours.hl_keyword_type_colour;
                case HL_KEYWORD_PP: return Editor.colours.hl_keyword_pp_colour;
                case HL_KEYWORD_RETURN: return Editor.colours.hl_keyword_return_colour;
                case HL_KEYWORD_ADAPTER: return Editor.colours.hl_keyword_adapter_colour;
                case HL_KEYWORD_LOOP: return Editor.colours.hl_keyword_loop_colour;
                case HL_STRING: return Editor.colours.hl_string_colour;
                case HL_NUMBER: return Editor.colours.hl_number_colour;
                case HL_MATCH: return Editor.colours.hl_match_colour;
  		        case HL_BACKGROUND_DEFAULT: return Editor.colours.hl_background_colour;
                default: return Editor.colours.hl_default_colour;
        }
}


// select the syntax highlight scheme depending on the filename
void editor_select_syntax_highlight(char *filename)
{
       for(unsigned int j = 0; j < HIGHLIGHT_DB_ENTRIES; j++) {
               struct editor_syntax *s = highlight_db+j;
               unsigned int i = 0;
               while(s->filematch[i]) {
                       char *p;
                       int patlen = strlen(s->filematch[i]);
                       if((p = strstr(filename, s->filematch[i])) != NULL) {
                               if(s->filematch[i][0] != '.' || p[patlen] == '\0') {
                                       Editor.syntax = s;
                                       return;
                               }
                       }
                       i++;
               }
       }
}



// Editor rows implementation


// update the rendered version and the syntax highlight of a row
void editor_update_row(editing_row *row)
{
        int tabs = 0, nonprint = 0, j, index;

        //create a version of the row that we can directly print on the screen
        free(row->rendered_chars);
        for(j = 0; j < row->size; j++)
                if( row->chars[j] == TAB) tabs++;

        row->rendered_chars = malloc(row->size + tabs*8 + nonprint*9 + 1);
        index = 0;
        for(j = 0; j < row->size; j++) {
                if(row->chars[j] == TAB) {
                        row->rendered_chars[index++] = ' ';
                        while((index+1) % 8 != 0) row->rendered_chars[index++] = ' ';
                } else {
                        row->rendered_chars[index++] = row->chars[j];
                }
        }
        row->rendered_size = index;
        row->rendered_chars[index] = '\0';

        // update the syntax highlighting of the row
        editor_update_syntax(row);
}


// insert row
void editor_insert_row(int at, char *s, size_t length)
{
        if(at > Editor.num_of_rows) return;
        Editor.row = realloc(Editor.row, sizeof(editing_row)*(Editor.num_of_rows+1));
        if(at != Editor.num_of_rows) {
                memmove(Editor.row + at + 1, Editor.row + at, sizeof(Editor.row[0])*(Editor.num_of_rows-at));
                for(int j = at+1; j <= Editor.num_of_rows; j++) Editor.row[j].index++;
        }

        Editor.row[at].size = length;
        Editor.row[at].chars = malloc(length+1);
        memcpy(Editor.row[at].chars, s, length+1);
        Editor.row[at].hl = NULL;
        Editor.row[at].hl_open_comment = 0;
        Editor.row[at].rendered_chars = NULL;
        Editor.row[at].rendered_size = 0;
        Editor.row[at].index = at;
        editor_update_row(Editor.row+at);
        Editor.num_of_rows++;
        Editor.dirty++;
}



// free the rows heap allocated data
void editor_free_row(editing_row *row)
{
        free(row->rendered_chars);
        free(row->chars);
        free(row->hl);
}


// remove row at specified position
void editor_delete_row(int at)
{
        editing_row *row;

        if(at >= Editor.num_of_rows) return;
        row = Editor.row+at;
        editor_free_row(row);
        memmove(Editor.row+at, Editor.row+at+1, sizeof(Editor.row[0])*(Editor.num_of_rows-at-1));
        for(int j = at; j < Editor.num_of_rows-1; j++) Editor.row[j].index++;
        Editor.num_of_rows--;
        Editor.dirty++;
}


// turn editor rows into single heap allocated string
char *editor_rows_to_string(int *buflen)
{
        char *buf = NULL, *p;
        int totlen = 0;
        int j;

        // compute count of bytes
        for(j = 0; j < Editor.num_of_rows; j++)
                totlen += Editor.row[j].size+1; // +1 is for \n at end of every row
        *buflen = totlen;
        totlen++; // make space for null terminator

        p = buf = malloc(totlen);
        for(j = 0; j < Editor.num_of_rows; j++) {
                memcpy(p, Editor.row[j].chars, Editor.row[j].size);
                p += Editor.row[j].size;
                *p = '\n';
                p++;
        }
        *p = '\0';
        return buf;
}



// insert char at the specified position in row
void editor_row_insert_char(editing_row *row, int at, int c)
{
        if(at > row->size) {
                // pad the string with spaces if the insert location is outside the current length by more than one char
                int padlen = at-row->size;

                row->chars = realloc(row->chars, row->size+padlen+2); // +2 for null term and newline
                memset(row->chars+row->size, ' ', padlen);
                row->chars[row->size+padlen+1] = '\0';
                row->size += padlen+1;
        } else {
                // in the middle of string so make empty space for char
                row->chars = realloc(row->chars, row->size+2);
                memmove(row->chars+at+1, row->chars+at, row->size-at+1);
                row->size++;
        }
        row->chars[at] = c;
        editor_update_row(row);
        Editor.dirty++;
}


// append the string s to the end of the row
void editor_row_append_string(editing_row *row, char *s, size_t len)
{
        row->chars = realloc(row->chars, row->size+len+1);
        memcpy(row->chars+row->size, s, len);
        row->size += len;
        row->chars[row->size] = '\0';
        editor_update_row(row);
        Editor.dirty++;
}


// delete the character at the offset 'at' from the specified row
void editor_row_delete_char(editing_row *row, int at)
{
        if(row->size <= at) return;
        memmove(row->chars+at, row->chars+at+1, row->size-at);
        editor_update_row(row);
        row->size--;
        Editor.dirty++;
}

// insert char at current prompt position
void editor_insert_char(int c)
{
        int filerow = Editor.row_offset+Editor.cursor_y;
        int filecol = Editor.column_offset+Editor.cursor_x;
        editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : &Editor.row[filerow];

        // if the row where rthe cursor is currently located does not exist, add enough empty rows as needed
        if(!row) {
                while(Editor.num_of_rows <= filerow)
                        editor_insert_row(Editor.num_of_rows, "", 0);
        }

        row = &Editor.row[filerow];
        editor_row_insert_char(row, filecol, c);
        if(Editor.cursor_x == Editor.screen_columns-1)
                Editor.column_offset++;
        else
                Editor.cursor_x++;
        Editor.dirty++;
}

char *get_indent_prefix(char *s, int max_length)
{
    int ptr = 0;
    while (s[ptr] == ' ' || s[ptr] == '\t') ptr++;
    if (ptr > max_length) ptr = max_length;
    char *res = (char*)malloc((ptr + 1) * sizeof(char));
    memcpy(res, s, ptr * sizeof(char));
    res[ptr] = '\0';
    return res;
}

char *set_indent_prefix(char *text, char *prefix)
{
    int result_length = strlen(text) + strlen(prefix);
    int ptr = 0;
    while (text[ptr] == ' ' || text[ptr] == '\t') ptr++;
    result_length -= ptr;
    char *res = (char*)malloc((result_length + 1) * sizeof(char));
    res[0] = '\0';
    res = strcat(res, prefix);
    res = strcat(res, text + ptr);
    return res;
}

/*
 * This function copies the current line to a separate buffer.
 */
void editor_yank_row() {
    int filerow = Editor.row_offset+Editor.cursor_y;

    editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : &Editor.row[filerow];

    if (row) {
        Editor.yank_buffer_len = row->size+1;
        if (!Editor.yank_buffer) {
            if ((Editor.yank_buffer = (char*) malloc(Editor.yank_buffer_len)) != NULL ) {
                memcpy(Editor.yank_buffer, row->chars, Editor.yank_buffer_len);
            } else {
                perror("Error allocating memory.");
                exit(1);
            }
        } else {
            if ((Editor.yank_buffer = (char*) realloc(Editor.yank_buffer, Editor.yank_buffer_len)) != NULL ) {
                memcpy(Editor.yank_buffer, row->chars, Editor.yank_buffer_len);
            } else {
                perror("Error allocating memory.");
                exit(1);
            }
        }
        editor_delete_row(filerow);
    }
}

void editor_paste_row() {
    int filerow = Editor.row_offset+Editor.cursor_y;
    editor_insert_row(filerow, Editor.yank_buffer, Editor.yank_buffer_len-1); // note -1 to account for the null terminator
}

/* Inserting a newline is slightly complex as we have to handle inserting a
 *  * newline in the middle of a line, splitting the line as needed. */
void editor_insert_newline(void)
{
        int filerow = Editor.row_offset+Editor.cursor_y;
        int filecol = Editor.column_offset+Editor.cursor_x;
        editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : &Editor.row[filerow];

        if (!row) {
                if(filerow == Editor.num_of_rows) {
                        editor_insert_row(filerow, "", 0);
                        goto fixcursor;
                }
                return;
        }
        char *indent_prefix = NULL;
        if (Editor.indent)
        {
            indent_prefix = get_indent_prefix(row->chars, filecol + 1);
        }
        else
        {
            indent_prefix = get_indent_prefix(row->chars, 0);
        }
        // if the cursor is over the line size then we conceptually think its just over the last character
        if(filecol >= row->size) filecol = row->size;
        if(filecol == 0) {
                editor_insert_row(filerow, indent_prefix, strlen(indent_prefix));
        } else {
                // we are in the middle of a line, split it between two rows
                char *new_row = set_indent_prefix(row->chars+filecol, indent_prefix);
                editor_insert_row(filerow+1, new_row, strlen(new_row));
                free(new_row);
                row = &Editor.row[filerow];
                row->chars[filecol] = '\0';
                row->size = filecol;
                editor_update_row(row);
        }
fixcursor:
        if(Editor.cursor_y == Editor.screen_rows-1)
                Editor.row_offset++;
        else
                Editor.cursor_y++;
        Editor.cursor_x = strlen(indent_prefix) % Editor.screen_columns;
        Editor.column_offset = strlen(indent_prefix) / Editor.screen_columns;
        free(indent_prefix);
}



// delete the char at the current position
void editor_delete_char()
{
        int filerow = Editor.row_offset+Editor.cursor_y;
        int filecol = Editor.column_offset+Editor.cursor_x;
        editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : &Editor.row[filerow];

        if(!row || (filecol == 0 && filerow == 0)) return;
        if(filecol == 0) {
                /* Handle the case of column 0, we need to move the current line
                 *          * on the right of the previous one. */
                filecol = Editor.row[filerow-1].size;
                editor_row_append_string(&Editor.row[filerow-1], row->chars, row->size);
                editor_delete_row(filerow);
                row = NULL;
                if(Editor.cursor_y == 0)
                        Editor.row_offset--;
                else
                        Editor.cursor_y--;
                Editor.cursor_x = filecol;
                if(Editor.cursor_x >= Editor.screen_columns) {
                        int shift = (Editor.screen_columns-Editor.cursor_x)+1;
                        Editor.cursor_x -= shift;
                        Editor.column_offset += shift;
                }
        }

        else {
                editor_row_delete_char(row, filecol-1);
                if(Editor.cursor_x == 0 && Editor.column_offset)
                        Editor.column_offset--;
                else
                        Editor.cursor_x--;
        }
        if(row) editor_update_row(row);
        Editor.dirty++;
}


void parse_argument(char *arg)
{
    int ptr = 0;
    while (arg[ptr] != '\0')
    {
        switch (arg[ptr])
        {
            case 'l':
                Editor.line_numbers = 1;
                break;
            case 'i':
                Editor.indent = 1;
                break;
            default:
                // TODO: warning of unrecognised argument
                break;
        }
        ptr++;
    }
}
// load the specified progam in the editor memory
int editor_open(char *filename)
{
        FILE *fp;
        Editor.dirty = 0;
        free(Editor.filename);
        Editor.filename = strdup(filename);

        fp = fopen(filename, "r");
        if(!fp) {
                if(errno != ENOENT) {
                        perror("Opening file");
                        exit(1);
                }
                return 1;
        }

        char *line = NULL;
        size_t linecap = 0;
        ssize_t linelen;
        while((linelen = getline(&line, &linecap, fp)) != -1) {
                if(linelen && (line[linelen-1] == '\n' || line[linelen-1] == '\r'))
                        line[--linelen] = '\0';
                editor_insert_row(Editor.num_of_rows, line, linelen);
        }

        free(line);
        fclose(fp);
        Editor.dirty = 0;
        return 0;
}

// save the current file on disk
int editor_save(void)
{
        int len;
        char *buf = editor_rows_to_string(&len);
        int fd = open(Editor.filename, O_RDWR|O_CREAT, 0644);
        if(fd == -1) goto writeerr;

        /* Use truncate + a single write(2) call in order to make saving
         *      * a bit safer, under the limits of what we can do in a small editor. */
        if(ftruncate(fd, len) == -1) goto writeerr;
        if(write(fd, buf, len) != len) goto writeerr;

        close(fd);
        free(buf);
        Editor.dirty = 0;
        editor_set_status_message("%d bytes written on disk", len);
        return 0;

writeerr:
        free(buf);
        if(fd != -1) close(fd);
        editor_set_status_message("Cannot save! I/O error: %s", strerror(errno));
        return 1;
}




// TERMINAL UPDATING


void ab_append(struct append_buf *ab, char *s, int length)
{
        char *new = realloc(ab->b, ab->length+length);

        if(new == NULL) return;
        memcpy(new+ab->length, s, length);
        ab->b = new;
        ab->length += length;
}


void ab_free(struct append_buf *ab)
{
        free(ab->b);
}

void copy_to_clipboard(void)
{
    free(Editor.clipboard);
    int lx = Editor.cursor_x + Editor.column_offset;
    int ly = Editor.cursor_y + Editor.row_offset;
    int rx = Editor.selected_base_x;
    int ry = Editor.selected_base_y;
    if (ly > ry || (ly == ry && lx > rx))
    {
        int t = lx;
        lx = rx;
        rx = t;
        t = ly;
        ly = ry;
        ry = t;
    }
    int clipboard_length = 1;
    for (int i = ly; i <= ry; i++)
    {
        int left_border = 0, right_border = Editor.row[i].size - 1;
        if (i == ly) left_border = lx;
        if (i == ry && rx < right_border) right_border = rx;
        clipboard_length += right_border - left_border + 1;
        if (i != ry) clipboard_length++;
    }
    Editor.clipboard = (char*)malloc(clipboard_length * sizeof(char));
    int ptr = 0;
    for (int i = ly; i <= ry; i++)
    {
        int left_border = 0, right_border = Editor.row[i].size - 1;
        if (i == ly) left_border = lx;
        if (i == ry && rx < right_border) right_border = rx;
        for (int j = left_border; j <= right_border; j++)
        {
            Editor.clipboard[ptr] = Editor.row[i].chars[j];
            ptr++;
        }
        if (i != ry)
        {
            Editor.clipboard[ptr] = '\n';
            ptr++;
        }
    }
    Editor.clipboard[ptr] = '\0';
}
void paste_from_clipboard(void)
{
    if (Editor.clipboard == NULL) return;
    int length = strlen(Editor.clipboard);
    for (int i = 0; i < length; i++)
    {
        if (Editor.clipboard[i] == '\n')
        {
            editor_insert_newline();
        }
        else
        {
            editor_insert_char(Editor.clipboard[i]);
        }
    }
}
int is_char_selected(int x, int y)
{
    int lx = Editor.cursor_x + Editor.column_offset;
    int ly = Editor.cursor_y + Editor.row_offset;
    int rx = Editor.selected_base_x;
    int ry = Editor.selected_base_y;
    if (ly > ry || (ly == ry && lx > rx))
    {
        int t = lx;
        lx = rx;
        rx = t;
        t = ly;
        ly = ry;
        ry = t;
    }
    if (ly == ry)
    {
        return y == ly && x >= lx && x <= rx;
    }
    else if (y == ly)
    {
        return x >= lx;
    }
    else if (y == ry)
    {
        return x <= rx;
    }
    else
    {
        return y >= ly && y <= ry;
    }
}

// this function writes the whole screen using VT100 escape characters starting
// from the logical state of the editor in the global state 'Editor'
void editor_refresh_screen(void)
{
        int y;
        editing_row *row;
        char buf[32];
        struct append_buf ab = ABUF_INIT;

        ab_append(&ab, "\x1b[?25l", 6); //hide cursor
        ab_append(&ab, "\x1b[H", 3); // go home
        for(y = 0; y < Editor.screen_rows; y++) {
                int filerow = Editor.row_offset+y;
                ab_append(&ab, "\x1b[49m", 5);
                ab_append(&ab, "\x1b[39m", 5);

                if (Editor.line_numbers)
                {
                    sprintf(buf, LINE_NUMBER_FORMAT, filerow + 1);
                    ab_append(&ab, buf, strlen(buf));
                }
                if(filerow >= Editor.num_of_rows) {
                        if(Editor.num_of_rows == 0 && y == Editor.screen_rows/3) {
                                char welcome[80];
                                int welcomelen = snprintf(welcome, sizeof(welcome), "Bric editor -- version %s\x1b[0K\r\n", BRIC_VERSION);
                                int padding = (Editor.screen_columns-welcomelen)/2;
                                if(padding) {
                                        ab_append(&ab, "~", 1);
                                        padding--;
                                }

                                while(padding--) ab_append(&ab, " ", 1);
                                ab_append(&ab, welcome, welcomelen);
                        } else {
                                ab_append(&ab, "~\x1b[0K\r\n", 7);
                        }

                        continue;
                }
                row = &Editor.row[filerow];
                int len = row->rendered_size - Editor.column_offset;
                int current_colour = -1, background_colour = -1;
                if(len > 0) {
                        if(len > Editor.screen_columns) len = Editor.screen_columns;
                        char *c = row->rendered_chars+Editor.column_offset;
                        unsigned char *hl = row->hl+Editor.column_offset;
                        int j;
                        for (j = 0; j < len; j++) {
                                if (Editor.mode == SELECTION_MODE && is_char_selected(j + Editor.column_offset, filerow)) {
                                    if (background_colour == -1)
                                    {
                                        ab_append(&ab, "\x1b[47m", 5);
                                        background_colour = 47;
                                    }
                                    if (current_colour != 30)
                                    {
                                        ab_append(&ab, "\x1b[30m", 5);
                                        current_colour = 30;
                                    }
                                    ab_append(&ab, c+j, 1);

                                } else if(hl[j] == HL_NONPRINT) {
                                        if (background_colour != -1)
                                        {
                                            ab_append(&ab, "\x1b[49m", 5);
                                            background_colour = -1;
                                        }
                                        char sym;
                                        ab_append(&ab, "\x1b[7m", 4);
                                        if(c[j] <= 26)
                                                sym = '@'+c[j];
                                        else
                                                sym = '?';
                                        ab_append(&ab, &sym, 1);
                                        ab_append(&ab, "\x1b[0m", 4);
                                } else if (hl[j] == HL_NORMAL) {
                                        if (background_colour != -1)
                                        {
                                            ab_append(&ab, "\x1b[49m", 5);
                                            background_colour = -1;
                                        }
                                        if(current_colour != -1) {
                                                ab_append(&ab, "\x1b[39m", 5);
                                                current_colour = -1;
                                        }
                                        ab_append(&ab, c+j, 1);
                                } else {
                                        int colour = editor_syntax_to_colour(hl[j]);
                                        if (background_colour != -1)
                                        {
                                            ab_append(&ab, "\x1b[49m", 5);
                                            background_colour = -1;
                                        }
                                        if(colour != current_colour) {
                                                char buf[16];
                                                int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", colour);
                                                current_colour = colour;
                                                ab_append(&ab, buf, clen);
                                        }
                                        ab_append(&ab, c+j, 1);

                                }
                        }
                }
                ab_append(&ab, "\x1b[39m", 5);
                ab_append(&ab, "\x1b[0K", 4);
                ab_append(&ab, "\r\n", 2);
        }

        //create a rwo row status
        //first row;
        ab_append(&ab, "\x1b[0K", 4);
        ab_append(&ab, "\x1b[7m", 4);
        char status[80], rstatus[80];
        int len = snprintf(status, sizeof(status), "%.20s =- %d lines %s", Editor.filename, Editor.num_of_rows, Editor.dirty ? "(modified)" : "");
        int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", Editor.row_offset+Editor.cursor_y+1, Editor.num_of_rows);
        if(len > Editor.screen_columns) len = Editor.screen_columns;
        ab_append(&ab, status, len);
        while(len < Editor.screen_columns) {
                if(Editor.screen_columns -len == rlen) {
                        ab_append(&ab, rstatus, rlen);
                        break;
                } else {
                        ab_append(&ab, " ", 1);
                        len++;
                }
        }
        ab_append(&ab, "\x1b[0m\r\n", 6);

        //second row depends on the Editor.status_message and update time
        ab_append(&ab, "\x1b[0K", 4);
        int message_length = strlen(Editor.status_message);
        if(message_length && time(NULL)-Editor.status_message_time < 5)
                ab_append(&ab, Editor.status_message, message_length <= Editor.screen_columns ? message_length : Editor.screen_columns);

        // put cursor at its current position
        int j;
        int cursor_x = 1;
        if (Editor.line_numbers) cursor_x += LINE_NUMBER_LENGTH;
        int filerow = Editor.row_offset+Editor.cursor_y;
        editing_row *r = (filerow >= Editor.num_of_rows) ? NULL : &Editor.row[filerow];
        if(r) {
                int was_tab = 0;
                for(j = Editor.column_offset; j < (Editor.cursor_x+Editor.column_offset); j++) {
                        if(j < r->size && r->chars[j] == TAB)
                        {
                            cursor_x += 7-((cursor_x)%8);
                            was_tab = 1;
                        }
                        cursor_x++;
                }
                if (was_tab) cursor_x--;
        }
        snprintf(buf, sizeof(buf), "\x1b[%d;%dH", Editor.cursor_y + 1, cursor_x);
        ab_append(&ab, buf, strlen(buf));
        ab_append(&ab, "\x1b[?25h", 6);
        write(STDOUT_FILENO, ab.b, ab.length);
        ab_free(&ab);
}



// set an editor status message for the second line of the status at the end of the screen
void editor_set_status_message(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(Editor.status_message, sizeof(Editor.status_message), fmt, ap);
        va_end(ap);
        Editor.status_message_time = time(NULL);
}


// FINDING MODE


void editor_find(int fd)
{
        char query[BRIC_QUERY_LENGTH+1] = {0};
        int qlen = 0;
        int last_match = -1; // last line where match was found -1 for noe
        int find_next = 0; // if 1 searh nex if -1 search prev
        int saved_hl_line = -1; // no savewd highlight
        char *saved_hl = NULL;

#define FIND_RESTORE_HL do { \
        if(saved_hl) { \
                memcpy(Editor.row[saved_hl_line].hl, saved_hl, Editor.row[saved_hl_line].rendered_size); \
                saved_hl = NULL; \
        } \
} while (0)

        //save the cursor position to restore it later
        int saved_cursor_x = Editor.cursor_x, saved_cursor_y = Editor.cursor_y;
        int saved_column_offset = Editor.column_offset, saved_row_offset = Editor.row_offset;

        while(1) {
                editor_set_status_message("Search: %s (Use ESC/Arrows/Enter)", query);
                editor_refresh_screen();

                int c = editor_read_key(fd);
                if(c == DEL_KEY || c == CTRL_H || c == BACKSPACE) {
                        if(qlen != 0) query[--qlen] = '\0';
                        last_match = -1;
                } else if(c == ESC || c == ENTER) {
                        if(c == ESC) {
                                Editor.cursor_x = saved_cursor_x; Editor.cursor_y = saved_cursor_y;
                                Editor.column_offset = saved_column_offset; Editor.row_offset = saved_row_offset;
                        }
                        FIND_RESTORE_HL;
                        editor_set_status_message("");
                        return;
                } else if (c == ARROW_RIGHT || c == ARROW_DOWN) {
                        find_next = 1;
                } else if(c == ARROW_LEFT || c == ARROW_UP) {
                        find_next = -1;
                } else if(isprint(c)) {
                        if(qlen < BRIC_QUERY_LENGTH) {
                                query[qlen++] = c;
                                query[qlen] = '\0';
                                last_match = -1;
                        }
                }

                // search occurrence
                if(last_match == -1) find_next = 1;
                if(find_next) {
                        char *match = NULL;
                        int match_offset = 0;
                        int i, current = last_match;

                        for (i = 0; i < Editor.num_of_rows; i++) {
                                current += find_next;
                                if(current == -1) current = Editor.num_of_rows-1;
                                else if (current == Editor.num_of_rows) current = 0;
                                match = strstr(Editor.row[current].rendered_chars, query);
                                if(match) {
                                        match_offset = match-Editor.row[current].rendered_chars;
                                        break;
                                }
                        }
                        find_next = 0;


                        // Highlight
                        FIND_RESTORE_HL;

                        if(match) {
                                editing_row *row = &Editor.row[current];
                                last_match = current;
                                if(row->hl) {
                                        saved_hl_line = current;
                                        saved_hl = malloc(row->rendered_size);
                                        memcpy(saved_hl, row->hl, row->rendered_size);
                                        memset(row->hl+match_offset, HL_MATCH, qlen);
					 memset(row->hl+match_offset+qlen, HL_BACKGROUND_DEFAULT, qlen);
                                }
                                Editor.cursor_y = 0;
                                Editor.cursor_x = match_offset;
                                Editor.row_offset = current;
                                Editor.column_offset = 0;
                                //scroll horizontally as needed
                                if(Editor.cursor_x > Editor.screen_columns) {
                                        int diff = Editor.cursor_x - Editor.screen_columns;
                                        Editor.cursor_x -= diff;
                                        Editor.column_offset += diff;
                                }
                        }
                }
        }
}

// REPLACE MODE

void editor_find_replace(int fd)
{
	char query[BRIC_QUERY_LENGTH + 1] = { 0 };
	char replace_word[BRIC_QUERY_LENGTH + 1] = { 0 };
	int replace_len = 0;
	int qlen = 0;
	int last_match = -1; // last line where match was found -1 for now
	int find_next = 0; // if 1 search next if -1 search prev
	int saved_hl_line = -1; // no saved highlight
	char *current_input = query;
	int *current_input_len = &qlen;
	char *saved_hl = NULL;

#define FIND_RESTORE_HL do { \
        if(saved_hl) { \
                memcpy(Editor.row[saved_hl_line].hl, saved_hl, Editor.row[saved_hl_line].rendered_size); \
                saved_hl = NULL; \
        } \
} while (0)

	//save the cursor position to restore it later
	int saved_cursor_x = Editor.cursor_x, saved_cursor_y = Editor.cursor_y;
	int saved_column_offset = Editor.column_offset, saved_row_offset = Editor.row_offset;

	while (1) {
		editor_set_status_message("Search: %s Replace: %s (Use ESC/Tab/Arrows/Enter)", query, replace_word);
		editor_refresh_screen();

		int c = editor_read_key(fd);
		if (c == CTRL_H || c == BACKSPACE) {
			if (*current_input_len != 0) current_input[--(*current_input_len)] = '\0';
			last_match = -1;
		}
		else if (c == ESC) {
			if (c == ESC) {
				Editor.cursor_x = saved_cursor_x; Editor.cursor_y = saved_cursor_y;
				Editor.column_offset = saved_column_offset; Editor.row_offset = saved_row_offset;
			}
			FIND_RESTORE_HL;
			editor_set_status_message("");
			return;
		}
		else if (c == ARROW_RIGHT || c == ARROW_DOWN) {
			find_next = 1;
		}
		else if (c == ARROW_LEFT || c == ARROW_UP) {
			find_next = -1;
		}
		else if (c == TAB) {
			if (current_input == query) {
				current_input = replace_word;
				current_input_len = &replace_len;
			} else if(current_input == replace_word) {
				current_input = query;
				current_input_len = &qlen;
			}
		}
		else if (c == ENTER) {
			Editor.cursor_x += qlen;
			for (int i = 0; i < qlen; i++)
			{
				editor_delete_char();
			}
			for (int j = 0; j < replace_len; j++)
			{
				editor_insert_char(replace_word[j]);
			}
		}
		else if (isprint(c)) {
			if (qlen < BRIC_QUERY_LENGTH) {
				current_input[(*current_input_len)++] = c;
				current_input[(*current_input_len)] = '\0';
				last_match = -1;
			}
		}

		// search occurrence
		if (last_match == -1) find_next = 1;
		if (find_next) {
			char *match = NULL;
			int match_offset = 0;
			int i, current = last_match;

			for (i = 0; i < Editor.num_of_rows; i++) {
				current += find_next;
				if (current == -1) current = Editor.num_of_rows - 1;
				else if (current == Editor.num_of_rows) current = 0;
				match = strstr(Editor.row[current].rendered_chars, query);
				if (match) {
					match_offset = match - Editor.row[current].rendered_chars;
					break;
				}
			}
			find_next = 0;


			// Highlight
			FIND_RESTORE_HL;

			if (match) {
				editing_row *row = &Editor.row[current];
				last_match = current;
				if (row->hl) {
					saved_hl_line = current;
					saved_hl = malloc(row->rendered_size);
					memcpy(saved_hl, row->hl, row->rendered_size);
					memset(row->hl + match_offset, HL_MATCH, qlen);
					memset(row->hl + match_offset + qlen, HL_BACKGROUND_DEFAULT, qlen);
				}
				Editor.cursor_y = 0;
				Editor.cursor_x = match_offset;
				Editor.row_offset = current;
				Editor.column_offset = 0;
				//scroll horizontally as needed
				if (Editor.cursor_x > Editor.screen_columns) {
					int diff = Editor.cursor_x - Editor.screen_columns;
					Editor.cursor_x -= diff;
					Editor.column_offset += diff;
				}
			}
		}
	}
}



// EDITOR EVENT HANDLING

//handle cursor position change due to arrow key press
void editor_move_cursor(int key)
{
        int filerow = Editor.row_offset + Editor.cursor_y;
        int filecol = Editor.column_offset + Editor.cursor_x;
        int rowlen;
        editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : &Editor.row[filerow];

        switch(key) {
                case 'h':
                case ARROW_LEFT:
                        if(Editor.cursor_x == 0) {
                                if(Editor.column_offset)
                                        Editor.column_offset--;
                                else {
                                        if(filerow > 0) {
                                                Editor.cursor_y--;
                                                Editor.cursor_x = Editor.row[filerow-1].size;
                                                if(Editor.cursor_x > Editor.screen_columns-1) {
                                                        Editor.column_offset = Editor.cursor_x-Editor.screen_columns+1;
                                                        Editor.cursor_x = Editor.screen_columns-1;
                                                }
                                        }
                                }
                        } else {
                                Editor.cursor_x -= 1;
                        }
                        break;
                case 'l':
                case ARROW_RIGHT:
                        if(row && filecol < row->size) {
                                if(Editor.cursor_x == Editor.screen_columns-1) {
                                        Editor.column_offset++;
                                } else {
                                        Editor.cursor_x += 1;
                                }
                        } else if ( row && filecol == row->size) {
                                Editor.cursor_x = 0;
                                Editor.column_offset = 0;
                                if(Editor.cursor_y == Editor.screen_rows-1) {
                                        Editor.row_offset++;
                                } else {
                                    Editor.cursor_y += 1;
                                }
                        }
                        break;
                case 'k':
                case ARROW_UP:
                        if(Editor.cursor_y == 0) {
                                if(Editor.row_offset) Editor.row_offset--;
                        } else {
                                Editor.cursor_y -= 1;
                        }
                        break;
                case 'j':
                case ARROW_DOWN:
                        if(filerow < Editor.num_of_rows) {
                                if(Editor.cursor_y == Editor.screen_rows-1) {
                                        Editor.row_offset++;
                                } else {
                                        Editor.cursor_y += 1;
                                }
                        }
                        break;
        }

        //fix cursor_x if the current line doesn't have enough chars
        filerow = Editor.row_offset+Editor.cursor_y;
        filecol = Editor.column_offset+Editor.cursor_x;
        row = (filerow >= Editor.num_of_rows) ? NULL : &Editor.row[filerow];
        rowlen = row ? row->size : 0;
        if(filecol > rowlen) {
                Editor.cursor_x -= filecol-rowlen;
                if (Editor.cursor_x < 0) {
                        Editor.column_offset += Editor.cursor_x;
                        Editor.cursor_x = 0;
                }
        }
}



// GOTO
void editor_goto(int fd)
{
	char query[BRIC_QUERY_LENGTH+1] = {0};
	int qlen = 0;
	int line_number;
	int current_line;

	while(1) {
		current_line = Editor.row_offset + Editor.cursor_y + 1;
		editor_set_status_message("Goto line: %s (ESC/ENTER)", query);
		editor_refresh_screen();

		int c = editor_read_key(fd);
		if(c == DEL_KEY || c == BACKSPACE) {
			if(qlen != 0) query[--qlen] = '\0';
		}else if(c == ENTER || c == ESC) {
			if(c == ESC) {
				editor_set_status_message("");
				return;
			}
			if(line_number <= Editor.num_of_rows && line_number > 0) {
				if (current_line > line_number) {
					int diff = current_line - line_number;

          				while(diff > 0) {
						editor_move_cursor(ARROW_UP);
						diff--;
					}
					editor_set_status_message("");
				} else if(line_number > current_line) {
					int diff = line_number - current_line;

					while(diff > 0) {
						editor_move_cursor(ARROW_DOWN);
						diff--;
					}
					editor_set_status_message("");
				}
				editor_refresh_screen();
				return;
			} else {
				editor_set_status_message("Out of bounds");
				editor_refresh_screen();
				return;
			}
		} else if(isprint(c)) {
			if(qlen < BRIC_QUERY_LENGTH) {
				query[qlen++] = c;
				query[qlen] = '\0';
				sscanf(query, "%d", &line_number);
			}
		}
	}
}

void editor_harsh_quit()
{
        exit(0);
}

void editor_check_quit(int fd)
{
        char query[BRIC_QUERY_LENGTH+1] = {0};
        int qlen = 0;

        while(1) {
                editor_set_status_message("There are unsaved changes, quit? (y or n) %s", query);
                editor_refresh_screen();

                int c = editor_read_key(fd);
                if(c == DEL_KEY || c == BACKSPACE) {
                        if(qlen != 0) query[--qlen] = '\0';
                }else if(c == ENTER || c == ESC) {
                        if(c == ENTER) {
                                if (strcmp(query, "y") == 0) editor_harsh_quit();
                                else if(strcmp(query, "n") == 0) return;
                                else {
                                        editor_check_quit(fd);
                                        return;
                                }
                        }
                } else if(isprint(c)) {
                        if(qlen < BRIC_QUERY_LENGTH) {
                                query[qlen++] = c;
                                query[qlen] = '\0';
                        }
                }
        }
}

void editor_parse_command(int fd, char *query)
{
        if (strcmp(query, "q!") == 0) {
                editor_harsh_quit();
                return;
        } else if(strcmp(query, "w") == 0) {
                editor_save();
                return;
        } else if (strcmp(query, "wq") == 0) {
                editor_save();
                editor_harsh_quit();
                return;
        } else if (strcmp(query, "q") == 0) {
                if (Editor.dirty) {
                        editor_check_quit(fd);
                } else {
                        editor_harsh_quit();
                }
                return;
        }
}

void enter_command(int fd)
{
        char query[BRIC_QUERY_LENGTH+1] = {0};
	int qlen = 0;

	while(1) {
		editor_set_status_message(":%s", query);
		editor_refresh_screen();

		int c = editor_read_key(fd);
		if(c == DEL_KEY || c == BACKSPACE) {
			if(qlen != 0) query[--qlen] = '\0';
		}else if(c == ENTER || c == ESC) {
			if(c == ENTER) {
				editor_parse_command(fd, query);
				return;
			} else {
                                editor_set_status_message("");
                                return;
                        }
		} else if(isprint(c)) {
			if(qlen < BRIC_QUERY_LENGTH) {
				query[qlen++] = c;
				query[qlen] = '\0';
			}
		}
	}
}

void editor_process_key_press(int fd)
{
        static int quit_times = BRIC_QUIT_TIMES;

        int c = editor_read_key(fd);
        switch(Editor.mode) {
        case EDIT_MODE:
                switch(c) {
                        case ENTER:
                                editor_insert_newline();
                                break;
        		case CTRL_G:
        			editor_goto(fd);
        			break;
                      case CTRL_Q:
                                //quit if the file isnt dirty
                                if(Editor.dirty && quit_times) {
                                        editor_set_status_message("WARNING! File has unsaved changes." "Press Ctrl-Q %d more times to quit.", quit_times);
                                        quit_times--;
                                        return;
                                }
                                exit(0);
                                break;
                        case CTRL_S:
                                editor_save();
                                break;
                        case CTRL_Y:
                            editor_yank_row();
                            break;
                        case CTRL_P:
                          editor_paste_row();
                            break;
                        case CTRL_F:
                                editor_find(fd);
                                break;
        				case CTRL_R:
        						editor_find_replace(fd);
        						break;
                        case BACKSPACE:
                                editor_delete_char();
                                break;
                        case CTRL_H:
                        case DEL_KEY:
                                editor_move_cursor(ARROW_RIGHT);
                                editor_delete_char();
                                break;
                        case PAGE_UP:
                        case PAGE_DOWN:
                                if(c == PAGE_UP && Editor.cursor_y != 0)
                                        Editor.cursor_y = 0;
                                else if (c == PAGE_DOWN && Editor.cursor_y != Editor.screen_rows-1)
                                        Editor.cursor_y = Editor.screen_rows-1;
                                {
                                        int times = Editor.screen_rows;
                                        while(times--)
        									editor_move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
                                }
                                break;
                        case ARROW_UP:
                        case ARROW_DOWN:
                        case ARROW_LEFT:
                        case ARROW_RIGHT:
                                editor_move_cursor(c);
                                break;
                        case CTRL_L: // means refresh screen
                                break;
                        case CTRL_D:
                                if (Editor.mode == EDIT_MODE)
                                {
                                    Editor.mode = SELECTION_MODE;
                                    Editor.selected_base_x = Editor.cursor_x + Editor.column_offset;
                                    Editor.selected_base_y = Editor.cursor_y + Editor.row_offset;
                                    editor_set_status_message(selection_mode_message);
                                }
                                break;
                        case CTRL_C:
                                if (Editor.mode == SELECTION_MODE)
                                {
                                    copy_to_clipboard();
                                }
                                break;
                        case CTRL_V:
                                if (Editor.mode == EDIT_MODE)
                                {
                                    paste_from_clipboard();
                                }
                                break;
                        case ESC:
                                Editor.mode = NORMAL_MODE;
                                editor_set_status_message("Normal mode.");
                                break;
        		case END_KEY:
                            {
        					   int times = Editor.row->size - Editor.row->index;
                               while (times--)
                                    editor_move_cursor(ARROW_RIGHT);
                            }
                               break;

                        case TAB:
                            {
                                if(Editor.tab_length < 0)
                                {
                                        editor_insert_char(c);
                                }
                                else
                                {
                                    int i = 0;
                                    while(i < Editor.tab_length){
                                        editor_insert_char(' ');
                                        i++;
                                    }
                                }

                            }
                                break;
                        default:
                                editor_insert_char(c);
                                break;
                }
                break;
        case NORMAL_MODE:
                switch (c) {
                        case 'h':
                        case 'j':
                        case 'k':
                        case 'l':
                        case ARROW_LEFT:
                        case ARROW_UP:
                        case ARROW_RIGHT:
                        case ARROW_DOWN:
                                editor_move_cursor(c);
                                break;

                        case ':':
                                enter_command(fd);
                                break;
                        case 'e':
                                Editor.mode = EDIT_MODE;
                                break;

                }
                break;
        }
        quit_times = BRIC_QUIT_TIMES;
}



int editor_file_was_modified(void)
{
        return Editor.dirty;
}


void init_editor(void)
{
        Editor.cursor_x = 0;
        Editor.cursor_y = 0;
        Editor.row_offset = 0;
        Editor.column_offset = 0;
        Editor.num_of_rows = 0;
        Editor.row = NULL;
        Editor.dirty = 0;
        Editor.filename = NULL;
        Editor.syntax = NULL;
        Editor.colours.hl_comment_colour = 33;
        Editor.colours.hl_mlcomment_colour = 33;
        Editor.colours.hl_keyword_cond_colour = 36;
        Editor.colours.hl_keyword_type_colour = 32;
        Editor.colours.hl_keyword_pp_colour = 34;
        Editor.colours.hl_keyword_return_colour = 35;
        Editor.colours.hl_keyword_adapter_colour = 94;
        Editor.colours.hl_keyword_loop_colour = 36;
        Editor.colours.hl_string_colour = 31;
        Editor.colours.hl_number_colour = 34;
        Editor.colours.hl_match_colour = 101;
        Editor.colours.hl_background_colour = 49;
        Editor.colours.hl_default_colour = 37;
        Editor.mode = NORMAL_MODE;
        if(get_window_size(STDIN_FILENO, STDOUT_FILENO, &Editor.screen_rows, &Editor.screen_columns) == -1) {
                perror("Unable to query the screen for size (columns / rows)");
                exit(1);
        }
        if (Editor.line_numbers)
        {
            Editor.screen_columns -= LINE_NUMBER_LENGTH;
        }
        Editor.screen_rows -= 2; // get room for status bar
}

void load_config_file(void)
{
    struct passwd *user = getpwuid(getuid());
    char config_file[80];
    config_file[0] = 0;
    strcat(config_file, user->pw_dir);
    strcat(config_file, "/.bricrc");
    FILE *config = fopen(config_file, "r");
    if (config == NULL)
    {
        return;
    }
    char variable_name[60], value[60];
    while (fscanf(config, "set %s %s\n", variable_name, value) == 2)
    {
        if (strcmp(variable_name, "linenumbers") == 0)
        {
            if (strcmp(value, "true") == 0)
            {
                Editor.line_numbers = 1;
            }
            else
            {
                Editor.line_numbers = 0;
            }
        }
        else if (strcmp(variable_name, "indent") == 0)
        {
            if (strcmp(value, "true") == 0)
            {
                Editor.indent = 1;
            }
            else
            {
                Editor.indent = 0;
            }
        }
        else if (strcmp(variable_name, "hl_comment_colour") == 0)
        {
            Editor.colours.hl_comment_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_mlcomment_colour") == 0)
        {
            Editor.colours.hl_mlcomment_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_cond_colour") == 0)
        {
            Editor.colours.hl_keyword_cond_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_type_colour") == 0)
        {
            Editor.colours.hl_keyword_type_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_pp_colour") == 0)
        {
            Editor.colours.hl_keyword_pp_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_return_colour") == 0)
        {
            Editor.colours.hl_keyword_return_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_adapter_colour") == 0)
        {
            Editor.colours.hl_keyword_adapter_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_loop_colour") == 0)
        {
            Editor.colours.hl_keyword_loop_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_string_colour") == 0)
        {
            Editor.colours.hl_string_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_number_colour") == 0)
        {
            Editor.colours.hl_number_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_match_colour") == 0)
        {
            Editor.colours.hl_match_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_background_colour") == 0)
        {
            Editor.colours.hl_background_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_default_colour") == 0)
        {
            Editor.colours.hl_default_colour = atoi(value);
        }
        else if(strcmp(variable_name, "tab_length") == 0)
        {
            Editor.tab_length = atoi(value);
        }

    }
    fclose(config);
}

void close_editor(void)
{
    free(Editor.filename);
    for (int i = 0; i < Editor.num_of_rows; i++)
    {
        free(Editor.row[i].chars);
        free(Editor.row[i].rendered_chars);
        free(Editor.row[i].hl);
    }
    free(Editor.row);
    free(Editor.clipboard);
}

int main(int argc, char **argv)
{
        int file_arg = -1;
        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] != '-')
            {
                file_arg = i;
            }
        }
        if(file_arg == -1) {
                fprintf(stderr, "Usage: bric <filename>\n");
                exit(1);
        }
        init_editor();
        load_config_file();
        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                parse_argument(argv[i]);
            }
        }
        editor_select_syntax_highlight(argv[file_arg]);
        editor_open(argv[file_arg]);
        enable_raw_mode(STDIN_FILENO);
        editor_set_status_message(help_message);
        while(1) {
                editor_refresh_screen();
                editor_process_key_press(STDIN_FILENO);
        }
        close_editor();
        return 0;
}
