#ifndef HANDLING_H
#define HANDLING_H

#include <unistd.h>
#include <termios.h>

#include "editor.h"

void disable_raw_mode(int fd, struct termios *termios, struct editor_config *editor);

#endif
