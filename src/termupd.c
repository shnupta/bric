/**
 * Basic editor functionalities, like open, save etc.
 *
 */
#include "termupd.h"

extern struct editor_config Editor; /* Defined in bric.c, used in: editor_syntax_to_colour */
extern int line_number_length;
extern const char* line_number_format[];

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

        if (Editor.line_numbers && filerow < Editor.num_of_rows)
        {
            /*if(filerow) {
                line_number_length = 3 + (int)log10((double)filerow);
            }*/
            sprintf(buf, line_number_format[line_number_length - 3], filerow + 1);
            ab_append(&ab, buf, strlen(buf));
            ab_append(&ab, ": ", 2);
        }
        if(filerow >= Editor.num_of_rows) {
            if(Editor.num_of_rows == 1 && y == Editor.screen_rows/3 && Editor.mode != INSERT_MODE && !Editor.dirty && Editor.newfile) {
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
        row = find_row(filerow);
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
    int len = snprintf(status, sizeof(status), "%.50s =- %d lines %s", Editor.filename, Editor.num_of_rows, Editor.dirty ? "(modified)" : "");
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
    if (Editor.line_numbers) cursor_x += line_number_length;
    int filerow = Editor.row_offset+Editor.cursor_y;
    editing_row *r = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);
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


