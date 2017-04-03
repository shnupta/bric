#include "bric.h"


static struct editor_config Editor;



// Low level terminal handling
 void disable_raw_mode(int fd)
{
        // dont bother checking the return value as its too late
        if (Editor.rawmode) {
                tcsetattr(fd, TCSAFLUSH, &orig_termios);
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
        if(!isatty(STDIN_FILENO)) goto fatal;
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
        if(nread = -1) exit(1);

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
                                else if(seq[0] == '0') {
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
                retval = get_cursor_pos(ifd, ofd, &original_row, &original_column);
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
        return c == '\0' || isspace(c) || strchr(",.()+_/*=~%[];", c) != NULL;
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
                                int kw2 = keywords[j][klen-1] == '|';
                                if(kw2) klen--;

                                if(!memcmp(p, keywords[j], klen) && is_separator(*(p+klen))) {
                                        // keyword
                                        memset(row->hl+i, kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
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
        if(row->hl_open_comment != oc && row->index+1 < Editor.num_of_rows)editor_update_syntax(&Editor.row[row->index+1]);
        row->hl_open_comment = oc;
}

// map thee syntax highlight types to terminal colours
int editor_syntax_to_colour(int highlight)
{
        switch(highlight) {
                case HL_COMMENT:
                case HL_MLCOMMENT: return 36; // cyan
                case HL_KEYWORD1: return 33; // yellow
                case HL_KEYWORD2: return 32; // green
                case HL_STRING: return 35; // magenta
                case HL_NUMBER: return 31; // red
                case HL_MATCH: return 34; // blue
                default: return 37; // white
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









