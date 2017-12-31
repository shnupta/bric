// Editor rows implementation
#include "erow_func.h"
#include "defs.h"
#include "synthigh.h"
#include <stdlib.h>
#include <string.h>         /* memcpy() */
#include <math.h>
#include <stdio.h>          /* perror() */

extern struct editor_config Editor; /* Defined in bric.c, global variable */
extern int line_number_length;

editing_row *find_row(int at)
{
    editing_row *ret;
    if (Editor.current != NULL)
        ret = Editor.current;
    else
        ret = Editor.row_head;
    while (ret != NULL && ret->index != at)
    {
        if (ret->index > at)
            ret = ret->prev;
        else if (ret->index < at)
            ret = ret->next;
    }
    if (ret != NULL)
        Editor.current = ret;
    return ret;
}

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
    //Editor.row = realloc(Editor.row, sizeof(editing_row)*(Editor.num_of_rows+1));

    editing_row *new = (editing_row*)malloc(sizeof(editing_row));

    /*memmove(Editor.row + at + 1, Editor.row + at, sizeof(Editor.row[0])*(Editor.num_of_rows-at));
      for(int j = at+1; j <= Editor.num_of_rows; j++) Editor.row[j].index++;*/
    if (Editor.num_of_rows == 0)
    {
        Editor.row_head = new;
        Editor.row_tail = NULL;
        Editor.row_head->prev = Editor.row_head->next = NULL;
    }
    else if (at == Editor.num_of_rows)
    {
        if (Editor.row_tail == NULL)
        {
            Editor.row_tail = new;
            new->prev = Editor.row_head;
            new->next = NULL;
            Editor.row_head->next = new;
        }
        else
        {
            Editor.row_tail->next = new;
            new->prev = Editor.row_tail;
            Editor.row_tail = Editor.row_tail->next;
            new->next = NULL;
        }
    }
    else
    {
        if (at == 0)
        {
            Editor.row_head->prev = new;
            new->next = Editor.row_head;
            Editor.row_head = Editor.row_head->prev;
        }
        else
        {
            editing_row *row = find_row(at);
            new->next = row;
            new->prev = row->prev;
            if (new->prev)
                new->prev->next = new;
            row->prev = new;
        }
        for (editing_row *i = new->next; i != NULL; i = i->next)
        {
            ++i->index;
        }
    }

    new->size = length;
    new->chars = malloc(length+1);
    memcpy(new->chars, s, length+1);
    new->hl = NULL;
    new->hl_open_comment = 0;
    new->rendered_chars = NULL;
    new->rendered_size = 0;
    new->index = at;
    editor_update_row(new/*Editor.row+at*/);
    ++Editor.num_of_rows;
    if(Editor.num_of_rows)
        line_number_length = 3 + (int)log10((double)Editor.num_of_rows);
//	printf("%d", line_number_length);
    Editor.dirty++;
    Editor.current = new;
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
    //row = Editor.row+at;
    row = find_row(at);

    //memmove(Editor.row+at, Editor.row+at+1, sizeof(Editor.row[0])*(Editor.num_of_rows-at-1));
    if (Editor.row_head == row)
    {
        Editor.row_head = row->next;
    }
    if (Editor.row_tail == row)
    {
        Editor.row_tail = row->prev;
    }
    if (row->prev)
        row->prev->next = row->next;
    if (row->next)
        row->next->prev = row->prev;

    for (editing_row *i = row->next; i != NULL; i = i->next)
    {
        --i->index;
    }

    editor_free_row(row);
    //for(int j = at; j < Editor.num_of_rows-1; j++) Editor.row[j].index++;

    --Editor.num_of_rows;
    if(Editor.num_of_rows) {
        line_number_length = 3 + (int)log10((double)Editor.num_of_rows);
    }
    else {
        line_number_length = 3;
    }
    ++Editor.dirty;
}

// turn editor rows into single heap allocated string
char *editor_rows_to_string(int *buflen)
{
    char *buf = NULL, *p;
    int totlen = 0;
    editing_row *j;

    // compute count of bytes
    for(j = Editor.row_head; j != NULL; j = j->next)
        totlen += j->size+1; // +1 is for \n at end of every row
    *buflen = totlen;
    totlen++; // make space for null terminator

    p = buf = malloc(totlen);
    for(j = Editor.row_head; j != NULL; j = j->next) {
        memcpy(p, j->chars, j->size);
        p += j->size;
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
    editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);

    // if the row where the cursor is currently located does not exist, add enough empty rows as needed
    if(!row) {
        while(Editor.num_of_rows <= filerow)
            editor_insert_row(Editor.num_of_rows, "", 0);
        row = find_row(filerow);
    }

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

int editor_copy_row() {
    int filerow = Editor.row_offset+Editor.cursor_y;

    editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);

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
        return 1;
    }
    return 0;
}

void editor_yank_row() {
    if(editor_copy_row()) {
        int filerow = Editor.row_offset+Editor.cursor_y;
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
    editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);
    char *indent_prefix = NULL;

    if (!row) {
        if(filerow == Editor.num_of_rows) {
            editor_insert_row(filerow, "", 0);
            goto fixcursor;
        }
        return;
    }

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
        row = find_row(filerow);
        row->chars[filecol] = '\0';
        row->size = filecol;
        editor_update_row(row);
    }

    fixcursor:
    if(Editor.cursor_y == Editor.screen_rows-1)
        Editor.row_offset++;
    else
        Editor.cursor_y++;

    if(indent_prefix) {

        Editor.cursor_x = strlen(indent_prefix) % Editor.screen_columns;
        Editor.column_offset = strlen(indent_prefix) / Editor.screen_columns;
        free(indent_prefix);
    }
}



// delete the char at the current position
void editor_delete_char()
{
    int filerow = Editor.row_offset+Editor.cursor_y;
    int filecol = Editor.column_offset+Editor.cursor_x;
    editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);

    if(!row || (filecol == 0 && filerow == 0)) return;
    if(filecol == 0) {
        /* Handle the case of column 0, we need to move the current line
         *          * on the right of the previous one. */
        filecol = row->prev->size;
        editor_row_append_string(row->prev, row->chars, row->size);
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


