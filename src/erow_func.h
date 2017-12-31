/* Editor Rows Implementation */
#ifndef _EROW_FUNC_H

#define _EROW_FUNC_H

#include "defs.h" /* To let the compiler know what is editing_row */

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

char *get_indent_prefix(char *s, int max_length);

char *set_indent_prefix(char *text, char *prefix);

void editor_insert_newline(void);

void editor_delete_char(void);

#endif