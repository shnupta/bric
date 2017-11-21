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
#include <pwd.h>
#include <assert.h>
#include <math.h>
#include <signal.h>

#include "editor.h"

// FILE LOCKING
#include "locking.h"

// LOW-LEVEL TERMINAL HANDLING
#include "handling.h"

#include "modules/syntax/syntax.h"
#include "modules/tag/tagfuncs.h"

#define INSERT_MODE 0
#define SELECTION_MODE 1
#define NORMAL_MODE 2

const char help_message[] = "Normal mode.";
const char selection_mode_message[] = "Selection mode: ESC = exit | arrows = select | Ctrl-C = copy";
// this represents the current single line of the file that we are editing


// Syntax highlighting!!!

int is_separator(int c);

int editing_row_has_open_comment(editing_row *row);

void editor_update_syntax(editing_row *row);

int editor_syntax_to_colour(int highlight); //maps the syntax highlight to the terminal colours

void editor_select_syntax_highlight(char *filename); // select the correct highlight scheme based on filetype



// Editor Rows Implementation

editing_row *find_row(int at);

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

int editor_copy_row();

void editor_yank_row();

void editor_paste_row();

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

void ab_append(struct append_buf *ab, char *s, int length);

void ab_free(struct append_buf *ab);

void editor_refresh_screen(void); // this writes the whole screen using VT100 escape characters

void editor_set_status_message(const char *fmt, ...);




// Find Mode!!

#define BRIC_QUERY_LENGTH 256

void editor_find(int fd);

void editor_goto(int linenumber);

// Editor events handling

void editor_move_cursor(int key); // handle cursor position change due to arrow key press

#define BRIC_QUIT_TIMES 3

//#define LINE_NUMBER_LENGTH 7

//#define LINE_NUMBER_FORMAT "%5d: "

#define TAB_LENGTH 4 // TODO: make it changable

void editor_process_key_press(int fd);

int editor_file_was_modified(void);

void init_editor(void);

void editor_start(char *filename);

#endif
