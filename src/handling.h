#ifndef HANDLING_H
#define HANDLING_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "editor.h"

void disable_raw_mode(int fd, struct termios *termios, struct editor_config *editor);
int editor_read_key(int fd);
int get_cursor_pos(int ifd, int ofd, int *rows, int *columns);

#endif
