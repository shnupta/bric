/*
 * handling.h
 *
 * Description: This file contents the functions used to handle the low-level terminal.
 *
 * Author:      Casey Williams <williamshoops96@gmail.com>
 * Rewriter:   Ivan Botero <ivan.botero@protonmail.ch>
 *
 * Copyright 2018 Bric Team <https://github.com/shnupta/bric>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
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

void disable_raw_mode (int fd, struct termios *termios,
                       struct editor_config *editor);
void editor_at_exit (void);
int enable_raw_mode (int fd, struct termios *termios,
                     struct editor_config *editor);
int editor_read_key (int fd);
int get_cursor_pos (int ifd, int ofd, int *rows, int *columns);
int get_window_size (int ifd, int ofd, int *rows, int *columns);

#endif
