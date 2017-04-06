#ifndef _BRIC_H
#define _BRIC_H

#define BRIC_VERSION "0.0.1"

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <time.h>

#include "modules/syntax/syntax.h"



// this represents the current single line of the file that we are editing
typedef struct editing_row {
        int index;              // the index of the row in the file
        int size;               // size of the row (not including null terminator)
        int rendered_size;      // size of the rendered row
        char *chars;             // the content of the row
        char *rendered_chars;    // the content of the rendered row
        unsigned char *hl;      // syntax highlighting for each character in the rendered row
        int hl_open_comment;    // the row had an open comment 
} editing_row;



// configuration structure for the editor
struct editor_config {
        int cursor_x, cursor_y;         // the x and y of the cursor
        int row_offset;                 // offset of the displayed row
        int column_offset;              // offset of the displayed column
        int screen_rows;                // number of rows that can be shown
        int screen_columns;             // number of columns that can be shown
        int num_of_rows;                // number of rows
        int rawmode;                    // is terminal raw mode enabled?
        editing_row *row;               // the rows
        int dirty;                      // if the file is modified but not saved
        char *filename;                 // currently open filename
        char status_message[80];
        time_t status_message_time;
        struct editor_syntax *syntax;   // current syntaxt highlighting
};

#define CTRL_KEY(k) ((k) & 0x1f)

enum KEY_ACTION {
        KEY_NULL = 0,
	 CTRL_G = CTRL_KEY('g'),
        CTRL_C = 3,
        CTRL_D = 3,
        CTRL_F = 6,
        CTRL_H = 8,
        TAB = 9,
        CTRL_L = 12,
        ENTER = 13,
        CTRL_Q = 17,
        CTRL_S = 19,
        CTRL_U = 21,
        ESC = 27,
        BACKSPACE = 127,
        ARROW_LEFT = 1000,
        ARROW_RIGHT,
        ARROW_UP,
        ARROW_DOWN,
        DEL_KEY,
        HOME_KEY,
        END_KEY,
        PAGE_UP,
        PAGE_DOWN
};





// Low level terminal handling


void disable_raw_mode(int fd);

void editor_at_exit(void); // called at exit to avoid remaining in raw mode

int enable_raw_mode(int fd); // lets us use terminal in raw mode

int editor_read_key(int fd); // read a key from the terminal put in raw mode

int get_cursor_pos(int ifd, int ofd, int *rows, int *columns); //using the ESC [6n escape sequence to query the cursor position and return it in *rows and *columns. 

int get_window_size(int ifd, int ofd, int *rows, int *columns); // try to get the number of columns in the current terminal




// Syntax highlighting!!!

int is_separator(int c);

int editing_row_has_open_comment(editing_row *row); 

void editor_update_syntax(editing_row *row); 

int editor_syntax_to_colour(int highlight); //maps the syntax highlight to the terminal colours

void editor_select_syntax_highlight(char *filename); // select the correct highlight scheme based on filetype



// Editor Rows Implementation

void editor_update_row(editing_row *row); //update the rendered version and the syntax highlighting of a row

void editor_insert_row(int at, char *s, size_t length); // insert a row at the specified position, shifting the other rows to the bottom if needed

void editor_free_row(editing_row *row); //free the current rows heap allocated data

void editor_delete_row(int at); // remove row at the specified position

char *editor_rows_to_string(int *buflen); //turn all rows to a single, heap-allocated string. The nreturn the pointer to the string and populate the interger pointed to by *buflen with the size of the string

void editor_row_insert_char(editing_row *row, int at, int c); // insert a character at the specified position in a row

void editor_row_append_string(editing_row *row,char *s, size_t len); // append the string s to the end of a row

void editor_row_delete_char(editing_row *row, int at); // delete the character at the offset at from the specified row

void editor_insert_char(int c); // insert specified char at current prompt position

void editor_insert_newline(void); // insert a new line

void editor_delete_char(); // delete the char at the current prompt position

int editor_open(char *filename); // load the specified program in the editor memory

int editor_save(void); //save the current file on the disk



// Terminal updating stuff


// this is a very simple append buffer struct, it is a heap allocated 
// string where we can append to. this is useful in order to write all
//the escape sequences in a buffer and flush them to the standard 
//output in a single call - to avoid flickering
struct append_buf {
        char *b;
        int length;
};

#define ABUF_INIT {NULL, 0}

void ab_append(struct append_buf *ab, const char *s, int length);

void ab_free(struct append_buf *ab);

void editor_refresh_screen(void); // this writes the whole screen using VT100 escape characters

void editor_set_status_message(const char *fmt, ...); 




// Find Mode!!

#define BRIC_QUERY_LENGTH 256

void editor_find(int fd);



// Editor events handling

void editor_move_cursor(int key); // handle cursor position change due to arrow key press

#define BRIC_QUIT_TIMES 3

void editor_process_key_press(int fd);

int editor_file_was_modified(void);

void init_editor(void);



#endif


