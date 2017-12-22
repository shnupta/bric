/**
 * File:        handling.h
 * 
 * Description: This file contents the functions used to handle the low-level terminal.
 * 
 * Author:      Casey Williams <williamshoops96@gmail.com>
 * Rewritter:   Ivan Botero <ivan.botero@protonmail.ch>
 */

#ifndef HANDLING_H
#define HANDLING_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "editor.h"

void disable_raw_mode(int fd, struct termios *termios, struct editor_config *editor);
void editor_at_exit(void);
int enable_raw_mode(int fd, struct termios *termios, struct editor_config *editor);
int editor_read_key(int fd);
int get_cursor_pos(int ifd, int ofd, int *rows, int *columns);
int get_window_size(int ifd, int ofd, int *rows, int *columns);

#endif
