#include "handling.h"

extern struct editor_config Editor;
extern struct termios orig_termios;

/**
 * Function:    disable_raw_mode()
 * 
 * Objective:   TODO: Write this item.
 * 
 * Arguments:   fd      <int>                       STDIN File descriptor.
 *              termios <struct termios *>          Original termios structure.
 *              editor  <struct editor_config *>    Editor configuration structure.
 * 
 * Return:      N/A
 * 
 */
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

/**
 * Function:    editor_at_exit()
 * 
 * Objective:   TODO: Write this item.
 * 
 * Arguments:   N/A
 * 
 * Return:      N/A
 * 
 */
void editor_at_exit(void)
{
    disable_raw_mode(STDIN_FILENO, &orig_termios, &Editor);
}

/**
 * Function:    enable_raw_mode()
 * 
 * Objective:   Read a key from the terminal put in raw mode.
 * 
 * Arguments:   fd      <int>                       STDIN File descriptor.
 *              termios <struct termios *>          TODO: Write this item.
 *              editor  <struct editor_config *>    TODO: Write this item.
 * 
 * Return:      <int>   TODO: Write this item.
 * 
 */
int enable_raw_mode(int fd, struct termios *termios, struct editor_config *editor)
{
    struct termios raw;

    if (editor->rawmode)
    {
        return 0; //already enabled
    }

    if (!isatty(fd))
    {
        goto fatal;
    }

    atexit(editor_at_exit);

    if (tcgetattr(fd, termios) == -1)
    {
        goto fatal;
    }

    raw = *termios; // modify the original mode

    /* input modes: no break, no CR to NL, no parity check, no strip char,
     *      * no start/stop output control. 
     */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // output modes - disable post processing
    raw.c_oflag &= ~(OPOST);

    //control modes - set 8 bit chars
    raw.c_cflag |= (CS8);

    //local modes, choing off, canonical off, no extended functions, no signal chars (, etc)
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    //control chars - set return condition: min number of bytes and a timer
    raw.c_cc[VMIN] = 0;  // return each byte, or zero for a timeout
    raw.c_cc[VTIME] = 1; //100ms timeout

    //put terminal in raw mode after flushing
    if (tcsetattr(fd, TCSAFLUSH, &raw) < 0)
    {
        goto fatal;
    }

    editor->rawmode = 1;
    return 0;

fatal:
    errno = ENOTTY;
    return -1;
}

/**
 * Function:    editor_read_key()
 * 
 * Objective:   Read a key from the terminal put in raw mode.
 * 
 * Arguments:   fd      <int>                       STDIN File descriptor.
 * 
 * Return:      <int>   TODO: Write this item.
 * 
 */
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

/**
 * Function:    get_cursor_pos()
 * 
 * Objective:   TODO: Write this item.
 * 
 * Arguments:   ifd     <int *>     TODO: Write this item.
 *              ofd     <int *>     TODO: Write this item.
 *              rows    <int *>     TODO: Write this item.
 *              columns <int *>     TODO: Write this item.
 * 
 * Return:      <int>   TODO: Write this item.
 * 
 */
int get_cursor_pos(int ifd, int ofd, int *rows, int *columns)
{
    char buf[32];
    unsigned int i = 0;

    // report cursor location
    if (write(ofd, "\x1b[6n", 4) != 4)
    {
        return -1;
    }

    // read the response: ESC [ rows; columns R
    while (i < sizeof(buf) - 1)
    {
        if (read(ifd, buf + i, 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';

    // parse it
    if (buf[0] != ESC || buf[1] != '[')
    {
        return -1;
    }
    if (sscanf(buf + 2, "%d;%d", rows, columns) != 2)
    {
        return -1;
    }

    return 0;
}

/**
 * Function:    get_window_size()
 * 
 * Objective:   TODO: Write this item.
 * 
 * Arguments:   ifd     <int *>     TODO: Write this item.
 *              ofd     <int *>     TODO: Write this item.
 *              rows    <int *>     TODO: Write this item.
 *              columns <int *>     TODO: Write this item.
 * 
 * Return:      <int>   TODO: Write this item.
 * 
 */
int get_window_size(int ifd, int ofd, int *rows, int *columns)
{
    struct winsize ws;

    if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        // ioctl() failed so query the terminal itself
        int original_row, original_column, retval;

        // get the initail position to store for later
        retval = get_cursor_pos(ifd, ofd, &original_row, &original_column);
        if (retval == -1)
        {
            goto failed;
        }

        // go to the right bottom margin and get position
        if (write(ofd, "\x1b[999C\x1b[999B", 12) != 12)
        {
            goto failed;
        }

        retval = get_cursor_pos(ifd, ofd, rows, columns);
        if (retval == -1)
        {
            goto failed;
        }

        // restore the cursor position
        char seq[32];
        snprintf(seq, 32, "\x1b[%d;%dH", original_row, original_column);
        if (write(ofd, seq, strlen(seq)) == -1)
        {
            // cant recover the cursor pos ....
        }

        return 0;
    }
    else
    {
        *columns = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }

failed:
    return -1;
}
