#include "handling.h"

void disable_raw_mode(int fd, struct termios *termios, struct editor_config *editor)
{
    // Don't bother checking the return value as its too late
    if (editor->rawmode)
    {
        tcsetattr(fd, TCSAFLUSH, &(*termios));
        write(fd, "\033[2J\033[H\033[?1049l", 15);
        editor->rawmode = 0;
    }

    return;
}