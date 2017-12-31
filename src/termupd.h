//
// Created by supreets51 on 9/12/17.
//

#ifndef BRIC_TERMUPD_H
#define BRIC_TERMUPD_H


#include "defs.h"
#include "erow_func.h"
#include "../bric.h"
#include "../modules/syntax/syntax.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

int is_char_selected(int x, int y);

#endif //BRIC_TERMUPD_H
