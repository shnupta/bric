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

// FILE LOCKING
#include "src/locking.h"

#include "modules/syntax/syntax.h"
#include "modules/tag/tagfuncs.h"

#define INSERT_MODE 0
#define SELECTION_MODE 1
#define NORMAL_MODE 2

extern const char help_message[]; // in bric.c
extern const char selection_mode_message[]; // in bric.c

/* Various data structures for use by the editor */
#include "src/defs.h"

/* Low level terminal handling */

#include "src/llth.h"

/* Syntax highlighting */

#include "src/synthigh.h"

/* Editor Rows Implementation */

#include "src/erow_func.h"

// Terminal updating stuff

#include "src/termupd.h"

// Find Mode and Replace mode!!!

#include "src/findmode.h"

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
