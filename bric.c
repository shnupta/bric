#include "bric.h"

static struct __current_file CurrentFile;
struct editor_config Editor; /* Remove static, to allwow source file splitting */
struct termios orig_termios; // so we can restore original at exit
int line_number_length = 3;
const char* line_number_format[] = {"%1d", "%2d", "%3d", "%4d", "%5d"};

const char help_message[] = "Normal mode.";
const char selection_mode_message[] = "Selection mode: ESC = exit | arrows = select | Ctrl-C = copy";

void sigwinch_handler()
{
	int old_columns = Editor.screen_columns;
	if (get_window_size(STDIN_FILENO, STDOUT_FILENO, &Editor.screen_rows, &Editor.screen_columns) == -1) {
                perror("Unable to query the screen for size (columns / rows)");
                exit(1);
        }

        if (Editor.line_numbers)
        {
		Editor.screen_columns -= line_number_length;
        }
        Editor.prev_char = ' ';
        Editor.screen_rows -= 2; // get room for status bar

	if (Editor.cursor_y >= Editor.screen_rows)
	{
		Editor.row_offset += Editor.cursor_y - Editor.screen_rows+1;
		Editor.cursor_y = Editor.screen_rows-1;
	}

	if (Editor.cursor_x >= Editor.screen_columns)
	{
		Editor.column_offset += Editor.cursor_x - Editor.screen_columns+1;
		Editor.cursor_x = Editor.screen_columns-1;
	}
	else
	{
		int delta = Editor.screen_columns - old_columns;
		if (delta > 0 && Editor.column_offset > 0)
		{
			int change = (delta > Editor.column_offset) ? Editor.column_offset : delta;
			Editor.column_offset -= change;
			Editor.cursor_x += change;
		}
	}

	editor_refresh_screen();
}


int numbers_only(const char *s)
{
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }

    return 1;
}


void parse_argument(char *arg)
{
    int ptr = 1;
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
                fprintf(stderr, "bric: invalid option -- '%c'\n", arg[ptr]);
                exit(1);
        }
        ptr++;
    }
}
// load the specified program in the editor memory
int editor_open(char *filename)
{
        FILE *fp;
        Editor.dirty = 0;
        fp = fopen(filename, "r");
        if(!fp) {
                if(errno != ENOENT) {
                        perror("Opening file");
                        exit(1);
                }
		/*Add a row if file is new*/
		editor_insert_row(Editor.num_of_rows, "", 0);
		Editor.dirty = 0;
		Editor.newfile = 1;
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
        Editor.newfile = 0;
        editor_set_status_message("%d bytes written on disk", len);
        return 0;

writeerr:
        free(buf);
        if(fd != -1) close(fd);
        editor_set_status_message("Cannot save! I/O error: %s", strerror(errno));
        return 1;
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
    editing_row *row = find_row(ly);
    for (int i = ly; i <= ry; i++)
    {
        int left_border = 0, right_border = row->size - 1;
        if (i == ly) left_border = lx;
        if (i == ry && rx < right_border) right_border = rx;
        clipboard_length += right_border - left_border + 1;
        if (i != ry) clipboard_length++;
        row = row->next;
    }
    Editor.clipboard = (char*)malloc(clipboard_length * sizeof(char));
    int ptr = 0;
    row = find_row(ly);
    for (int i = ly; i <= ry; i++)
    {
        int left_border = 0, right_border = row->size - 1;
        if (i == ly) left_border = lx;
        if (i == ry && rx < right_border) right_border = rx;
        for (int j = left_border; j <= right_border; j++)
        {
            Editor.clipboard[ptr] = row->chars[j];
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

// EDITOR EVENT HANDLING

//handle cursor position change due to arrow key press
void editor_move_cursor(int key)
{
        int filerow = Editor.row_offset + Editor.cursor_y;
        int filecol = Editor.column_offset + Editor.cursor_x;
        int rowlen;
        editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);

        switch(key) {
                case 'h':
                case ARROW_LEFT:
                        if(Editor.cursor_x == 0) {
                                if(Editor.column_offset)
                                        Editor.column_offset--;
                                else {
                                        if(filerow > 0) {
                                                Editor.cursor_y--;
                                                Editor.cursor_x = row->prev->size;
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
				/*Do not go beyond last row*/
				if(filerow == Editor.num_of_rows - 1) /*'filerow' indexing starts at 0*/ {
					break;
				}
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
                        if(filerow < Editor.num_of_rows - 1)/*Do not go beyond last row. 'filerow' indexing starts at 0*/ {
                                if(Editor.cursor_y == Editor.screen_rows-1) {
                                        Editor.row_offset++;
                                } else {
                                        Editor.cursor_y += 1;
                                }
                        }
                        break;
                case HOME_KEY:
                        Editor.cursor_x=0;
                        Editor.column_offset=0;
                        break;
                case END_KEY:
                        if(row->size>Editor.screen_columns-1)
                        {
                                Editor.cursor_x=Editor.screen_columns-1;
                                Editor.column_offset=row->size-(Editor.screen_columns+1)+1;
                        } else {
                                Editor.cursor_x=row->size-1;
                        }
                        break;
                case PAGE_UP:
                        if(Editor.cursor_y != 0) Editor.cursor_y = 0;

                        {
                        int times = Editor.screen_rows-2; // keep last two rows from previous view (like in vim)
                        while(times--)
                                editor_move_cursor(ARROW_UP);
                        }
                        break;

                case PAGE_DOWN:
                        if (Editor.cursor_y != Editor.screen_rows-1) Editor.cursor_y = Editor.screen_rows-1;

                        {
                        int times = Editor.screen_rows-2; // keep last two rows from previous view (like in vim)
                        while(times--)
                                editor_move_cursor(ARROW_DOWN);
                        }
                        break;
        }

        //fix cursor_x if the current line doesn't have enough chars
        filerow = Editor.row_offset+Editor.cursor_y;
        filecol = Editor.column_offset+Editor.cursor_x;
        row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);
        rowlen = row ? row->size : 0;
        if(filecol > rowlen) {
                Editor.cursor_x -= filecol-rowlen;
                if (Editor.cursor_x < 0) {
                        Editor.column_offset += Editor.cursor_x;
                        Editor.cursor_x = 0;
                }
        }
}


void editor_harsh_quit()
{

        // We check if file is locked and we'll unlock it
        if(is_file_locked(CurrentFile)){
          unlock_file(CurrentFile);
        }

        exit(EXIT_SUCCESS);
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

// TAG MOVEMENT FUNCTIONS

// Function to differentiate identifier names with language syntax
int char_check(char c) {
	if(c == ' ' || c == '\n' || c == '\t' || c == '(' || c == ')' || c == '{' || c == '}')
		return 0;
	if(c == '&' || c == '|' || c == '+' || c == '-' || c == '*' || c == '\\' || c == '/' )
		return 0;
	if(c == '^' || c == '%' || c == '=' || c == '[' || c == ']' || c == ';' || c == '>' || c == '<')
		return 0;
	if(c == '?' || c == ':' || c == '\'' || c == '\"' || c == '.' || c == ',' || c == '!')
		return 0;
	return 1;
}

// Function to get the key word (identifier) on which the cursor is positioned
char* get_key(void) {
	int i = 0;
	int filerow = Editor.row_offset + Editor.cursor_y;
	int filecol = Editor.column_offset + Editor.cursor_x;
	if(!char_check(find_row(filerow)->chars[filecol]))
		return "";
	while(filecol != -1 && char_check(find_row(filerow)->chars[filecol]))
		filecol--;
	filecol++;
	char *key = (char*)malloc(128 * sizeof(char));
	while(char_check(find_row(filerow)->chars[filecol])) {
		key[i] = find_row(filerow)->chars[filecol];
		i++; 
		filecol++;
	}
	key[i] = '\0';
	return key;
}

// Function to parse the tags file ex command and search for the required line in the file
int tagsearch(char *tosearch) {
	int dest_index = 0, source_index = 2, i = 0;
	char *parsedsearch = (char*)malloc(strlen(tosearch) * sizeof(char));
	while(tosearch[source_index] != '$') {
		if(tosearch[source_index] == '\\') {
			source_index++;
			continue;
		}
		parsedsearch[dest_index] = tosearch[source_index];
		source_index++;
		dest_index++;
	}
	parsedsearch[dest_index] = '\0';
	for(i = 0; i < Editor.num_of_rows; i++) {
		if(strstr(find_row(i)->chars, parsedsearch) != NULL)
			return i + 1;
	}
	return 0;
}

// Main function to handle tag movements
int handle_tag_movement(int where) {
	int linenumber = 0, i = 0;
	char *key = get_key();
	char tag_line[256], *tagname, *filename, *tosearch, *orig_filename;
	FILE *fp;
	int cursor_pos = Editor.cursor_y, cursor_offset = Editor.row_offset;
	tagdata tag_data;
	orig_filename = (char*)malloc(sizeof(char) * strlen(Editor.filename));
	strcpy(orig_filename, Editor.filename);
	if(where == MOVE_BACK) {
		if(isempty(&tag_stack)) {
			editor_set_status_message("at bottom of tag stack");
			return 1;
		}
		tag_data = pop(&tag_stack);
		linenumber = tag_data.linenumber;
		filename = (char*)malloc(sizeof(char) * strlen(tag_data.filename));
		strcpy(filename, tag_data.filename);
		if(strcmp(filename, Editor.filename)) {
                        if(Editor.dirty) {
                                editor_set_status_message("Unsaved changes. Can't proceed");
                                return 0;
                        }
                        editor_start(filename);
                }
		editor_goto(linenumber);
		/*Position the page so that the cursor remains in the same line*/
                for(i = 0; i < cursor_pos && Editor.row_offset + Editor.cursor_y < Editor.num_of_rows - 1; i++)
			editor_move_cursor(ARROW_DOWN);
                for(; i > 0; i--)
			editor_move_cursor(ARROW_UP);
                for(i = 0; i < cursor_pos && Editor.row_offset + Editor.cursor_y > 0; i++)
			editor_move_cursor(ARROW_UP);
                for(; i > 0; i--)
			editor_move_cursor(ARROW_DOWN);
                return 1;		
	}
	if(key[0] == '\0') {
		editor_set_status_message("No identifier under cursor");
		return 0;
	}
	fp = fopen("tags", "r");
	if(!fp) {
		editor_set_status_message("tag not found: %s", key);
		free(key);
		return 0;
	}
	while(fgets(tag_line, 256, fp) != NULL) {
		tagname = strtok(tag_line, "\t");
		if(strcmp(tagname, key))
			continue;
		filename = strtok(NULL, "\t");
		tosearch = strtok(NULL, "\t");
		if(strcmp(filename, Editor.filename)) {
			if(Editor.dirty) {
				editor_set_status_message("Unsaved changes. Can't proceed");
				return 0;				
			}
			editor_start(filename);
		}
		if(tosearch[0] == '/') {
			linenumber = tagsearch(tosearch);
		}
		else {
			sscanf(tosearch, "%d", &linenumber);
		}
		tag_data.linenumber = cursor_offset + cursor_pos + 1;
		strcpy(tag_data.filename, orig_filename);
		push(&tag_stack, tag_data);
		editor_goto(linenumber);
		/*Position the page so that the cursor remains in the same line*/
                for(i = 0; i < cursor_pos && Editor.row_offset + Editor.cursor_y < Editor.num_of_rows - 1; i++)
			editor_move_cursor(ARROW_DOWN);
                for(; i > 0; i--)
			editor_move_cursor(ARROW_UP);
                for(i = 0; i < cursor_pos && Editor.row_offset + Editor.cursor_y > 0; i++)
			editor_move_cursor(ARROW_UP);
                for(; i > 0; i--)
			editor_move_cursor(ARROW_DOWN);
		return 1;
	}
	editor_set_status_message("tag not found: %s", key);
	free(key);
	return 0;
}

void editor_parse_command(int fd, char *query)
{
        if (numbers_only(query)) {
                editor_goto(atoi(query));
                return;
        } else if (strcmp(query, "q!") == 0) {
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
        } else if (strcmp(query, "f") == 0) {
            editor_find(fd);
            return;
        } else if (strcmp(query, "fr") == 0) {
            editor_find_replace(fd);
            return;
        } else if (strcmp(query, "sm") == 0) {
            Editor.mode = SELECTION_MODE;
            Editor.selected_base_x = Editor.cursor_x + Editor.column_offset;
            Editor.selected_base_y = Editor.cursor_y + Editor.row_offset;
            editor_set_status_message(selection_mode_message);
            return;
        } else if (strcmp(query, "sp") == 0) {
            Editor.indent = 0;
            return;
        } else if (strcmp(query, "up") == 0) {
            Editor.indent = 1;
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
        int filerow = Editor.row_offset+Editor.cursor_y;
        int filecol = Editor.column_offset+Editor.cursor_x;
        switch(Editor.mode) {
        case INSERT_MODE:
                switch(c) {
                        case ENTER:
                                editor_insert_newline();
                                break;
                        case CTRL_G:
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
                                editor_move_cursor(PAGE_UP);
                                break;
                        case PAGE_DOWN:
                                editor_move_cursor(PAGE_DOWN);
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
                                if (Editor.mode == INSERT_MODE)
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
                                if (Editor.mode == INSERT_MODE)
                                {
                                    paste_from_clipboard();
                                }
                                break;
                        case ESC:
                                Editor.mode = NORMAL_MODE;
                                editor_set_status_message("Normal mode.");
                                if(filecol != 0)
                                    editor_move_cursor(ARROW_LEFT);
                                break;
                        case HOME_KEY:
                                editor_move_cursor(HOME_KEY);
                                break;
                        case END_KEY:
                        	editor_move_cursor(END_KEY);
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
                        case 'r':
                            if (Editor.prev_char == 'c') editor_copy_row();
                            if (Editor.prev_char == 'y') editor_yank_row();
                            if (Editor.prev_char == 'p') editor_paste_row();
                            if (Editor.prev_char == 'd') editor_delete_row(filerow);
                            break;
                        case 'p':
                            if (Editor.prev_char == 'c') paste_from_clipboard();
                            break;
                        case 'c':
                            if (Editor.prev_char == 'c') Editor.clipboard = "";
                            break;
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
                        case 'i':
                                Editor.mode = INSERT_MODE;
                                editor_set_status_message("Insert mode.");
                                break;
                        case 'I':
				editor_move_cursor(HOME_KEY);
				Editor.mode = INSERT_MODE;
				editor_set_status_message("Insert mode. ");
				break;
                        case 'o':
				Editor.mode = INSERT_MODE;
				editor_set_status_message("Insert mode.");
				editor_insert_row(filerow + 1, "", 0);
				editor_move_cursor(ARROW_DOWN);
				break;
                        case 'O':
				Editor.mode = INSERT_MODE;
				editor_set_status_message("Insert mode. ");
				editor_insert_row(filerow, "", 0);
				break;
                        case 'G':
                    		editor_goto(Editor.num_of_rows);
                    		break;
                        case 'g':
                    		editor_goto(1);
                    		break;
                        case '$':
				editor_move_cursor(END_KEY);
				break;
                        case '0':
				editor_move_cursor(HOME_KEY);
				break;
                        case 'a':     
				editor_move_cursor(ARROW_RIGHT);
				Editor.mode = INSERT_MODE;
				editor_set_status_message("Insert mode. ");
				break;
                        case 'A':
				Editor.mode = INSERT_MODE;
				editor_set_status_message("Insert mode. ");
				editor_move_cursor(END_KEY);
				editor_move_cursor(ARROW_RIGHT);
				break;
                        case HOME_KEY:
                                editor_move_cursor(HOME_KEY);
                                break;
                        case END_KEY:
                                editor_move_cursor(END_KEY);
                                break;
                        case PAGE_UP:
                                editor_move_cursor(PAGE_UP);
                                break;
                        case PAGE_DOWN:
                                editor_move_cursor(PAGE_DOWN);
                                break;
                        case CTRL_M:
                                handle_tag_movement(MOVE_AHEAD);
                                break;
                        case CTRL_N:
                                handle_tag_movement(MOVE_BACK);
                                break;
                }
                break;
            case SELECTION_MODE:
                switch(c) {
                    case 'h':
                    case 'j':
                    case 'k':
                    case 'l':
                    case ARROW_UP:
                    case ARROW_DOWN:
                    case ARROW_RIGHT:
                    case ARROW_LEFT:
                        editor_move_cursor(c);
                        break;

                    case ESC:
                        Editor.mode = NORMAL_MODE;
                        break;

                    case CTRL_C:
                        copy_to_clipboard();
                        break;
                }
        }

        Editor.prev_char = c;
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
        Editor.row_head = NULL;
	Editor.row_tail = NULL;
	Editor.current = NULL;
        Editor.dirty = 0;
        Editor.newfile = 0;
        Editor.filename = NULL;
        Editor.syntax = NULL;
	Editor.tab_length = TAB_LENGTH;
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
            Editor.screen_columns -= line_number_length;
        }
        Editor.prev_char = ' ';
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

    //free(Editor.row);
    editing_row *i = Editor.row_head;
    editing_row *prev;
    while (i != NULL)
    {
            prev = i;
            i = i->next;
            if (prev)
            {
                    free(prev->chars);
                    free(prev->rendered_chars);
                    free(prev->hl);
                    free(prev);
            }
    }
    free(Editor.clipboard);
}

// Given a filename, start editor with that file opened
void editor_start(char *filename) {
        free(Editor.filename);
        init_editor();
        Editor.filename = (char*)malloc(sizeof(char) * strlen(filename));
	strcpy(Editor.filename, filename);
        load_config_file();
        editor_select_syntax_highlight(filename);
        editor_open(filename);
        enable_raw_mode(STDIN_FILENO);
        editor_set_status_message(help_message);
}

int main(int argc, char **argv)
{
        signal(SIGWINCH, sigwinch_handler);
        init(&tag_stack);
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
        // We set the current file information
        set_current_file(argv[file_arg], &CurrentFile);

        // We check if current file is locked
        if(!is_file_locked(CurrentFile)){
                lock_file(CurrentFile);
        }else{
                fprintf(stderr, "The file has been locked, try to remove the locker!\n");
                return EXIT_FAILURE;
        }
        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                parse_argument(argv[i]);
            }
        }
        editor_start(argv[file_arg]);
        while(1) {
                editor_refresh_screen();
                editor_process_key_press(STDIN_FILENO);
        }

        close_editor();
        return EXIT_SUCCESS;
}
