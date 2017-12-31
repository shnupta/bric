//
// Created by supreets51 on 10/12/17.
// editor_goto and editor_find
//

#include "findmode.h"
#include "termupd.h"
#include <stdlib.h> /* For NULL macro */

extern struct editor_config Editor; /* Defined in bric.c */

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
                memcpy(find_row(saved_hl_line)->hl, saved_hl, find_row(saved_hl_line)->rendered_size); \
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

            editing_row *tmp = Editor.row_head;
            for (i = 0; i < Editor.num_of_rows; i++) {
                current += find_next;
                tmp = find_row(current);
                if(current == -1)
                {
                    current = Editor.num_of_rows-1;
                    tmp = Editor.row_tail;
                }
                else if (current == Editor.num_of_rows)
                {
                    current = 0;
                    tmp = Editor.row_head;
                }
                match = strstr(tmp->rendered_chars, query);
                if(match) {
                    match_offset = match-tmp->rendered_chars;
                    break;
                }
            }
            find_next = 0;


            // Highlight
            FIND_RESTORE_HL;

            if(match) {
                editing_row *row = find_row(current);
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


// GOTO
void editor_goto(int linenumber)
{
    int line_number = linenumber;
    int current_line;

    while(1) {
        current_line = Editor.row_offset + Editor.cursor_y + 1;
        editor_refresh_screen();

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
    }
}


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
                memcpy(find_row(saved_hl_line)->hl, saved_hl, find_row(saved_hl_line)->rendered_size); \
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
            editor_refresh_screen();
            return;
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

            editing_row *tmp = Editor.row_head;
            for (i = 0; i < Editor.num_of_rows; i++) {
                current += find_next;
                tmp = find_row(current);
                if (current == -1){
                    current = Editor.num_of_rows - 1;
                    tmp = Editor.row_tail;
                }
                else if (current == Editor.num_of_rows){
                    current = 0;
                    tmp = Editor.row_head;
                }

                match = strstr(tmp->rendered_chars, query);
                if (match) {
                    match_offset = match - tmp->rendered_chars;
                    break;
                }
            }
            find_next = 0;


            // Highlight
            FIND_RESTORE_HL;

            if (match) {
                editing_row *row = find_row(current);
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