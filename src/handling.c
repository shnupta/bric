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

int editor_read_key(int fd)
{
    int nread;
    char c, seq[3];
    while ((nread = read(fd, &c, 1)) == 0)
        ;

    if (nread == -1)
    {
        exit(1);
    }

    while (1)
    {
        switch (c)
        {
        case ESC: // escape sequence
            // if its an escape then we timeout here
            if (read(fd, seq, 1) == 0)
            {
                return ESC;
            }
            if (read(fd, seq + 1, 1) == 0)
            {
                return ESC;
            }
            // ESC [ sequences
            if (seq[0] == '[')
            {
                if (seq[1] >= '0' && seq[1] <= '9')
                {
                    // extended escape so read additional byte
                    if (read(fd, seq + 2, 1) == 0)
                    {
                        return ESC;
                    }
                    if (seq[2] == '~')
                    {
                        switch (seq[1])
                        {
                        case '3':
                            return DEL_KEY;
                        case '5':
                            return PAGE_UP;
                        case '6':
                            return PAGE_DOWN;
                        }
                    }
                }
                else
                {
                    switch (seq[1])
                    {
                    case 'A':
                        return ARROW_UP;
                    case 'B':
                        return ARROW_DOWN;
                    case 'C':
                        return ARROW_RIGHT;
                    case 'D':
                        return ARROW_LEFT;
                    case 'H':
                        return HOME_KEY;
                    case 'F':
                        return END_KEY;
                    }
                }
            }

            // ESC 0 sequences
            else if (seq[0] == 'O')
            {
                switch (seq[1])
                {
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
                }
            }
            break;

        default:
            return c;
        }
    }

    return c;
}