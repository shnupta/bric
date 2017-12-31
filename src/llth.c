/* Low level terminal handling */
/* Note: Have a look @ line 70: read returns ssize_t, but nsize is an int. Might lead to an error! */
/* @ Line 146: Consider using strtoi or strtol instead of sscanf, which doesnt return errors */

#include "llth.h"       /* Declares the functions used for terminal handling */
#include "defs.h"       /* To ensure proper definitions */
#include <termios.h>    /* Fir TCSAFLSUH flag */
#include <unistd.h>     /* For write(), in disable_raw_mode() */
#include <stdlib.h>     /* For atexit(), in enable_raw_mode() */
#include <errno.h>      /* errno, ENOTTY */
#include <stdio.h>      /* sscanf(), used in get_cursor_pos() */
#include <string.h>     /* for strlen() */
#include <sys/ioctl.h>  /* used in get_window_size() */

extern struct editor_config Editor; /* Defined in bric.c, global variable */
extern struct termios orig_termios; // so we can restore original at exit

void disable_raw_mode(int fd)
{
    // dont bother checking the return value as its too late
    if (Editor.rawmode) {
        tcsetattr(fd, TCSAFLUSH, &orig_termios);
        write(fd, "\033[2J\033[H\033[?1049l", 15);
        Editor.rawmode = 0;
    }
} //disable_raw_mode ends

void editor_at_exit(void)
{
    disable_raw_mode(STDIN_FILENO);
}

int enable_raw_mode(int fd)
{
    struct termios raw;

    if(Editor.rawmode) return 0; //already enabled
    if(!isatty(fd)) goto fatal;
    atexit(editor_at_exit);
    if(tcgetattr(fd, &orig_termios) == -1) goto fatal;

    raw = orig_termios; // modify the original mode
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     *      * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // output modes - disable post processing
    raw.c_oflag &= ~(OPOST);

    //control modes - set 8 bit chars
    raw.c_cflag |= (CS8);

    //local modes, choing off, canonical off, no extended functions, no signal chars (, etc)
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    //control chars - set return condition: min number of bytes and a timer
    raw.c_cc[VMIN] = 0; // return each byte, or zero for a timeout
    raw.c_cc[VTIME] = 1; //100ms timeout

    //put terminal in raw mode after flushing
    if(tcsetattr(fd, TCSAFLUSH, &raw) < 0) goto fatal;
    //if(write(fd, "\033[?1049h\033[2J\033[H", 15) != 15) goto fatal;
    Editor.rawmode = 1;
    return 0;

    fatal:
    errno = ENOTTY;
    return -1;
} //enable_raw_mode ends

/* read a key from terminal input in raw mode and handle */
int editor_read_key(int fd)
{
    int nread;
    char c, seq[3];
    while((nread = read(fd, &c, 1)) == 0);
    if(nread == -1) exit(1);

    while(1) {
        switch(c) {
            case ESC: // escape sequence
                // if its an escape then we timeout here
                if(read(fd, seq, 1) == 0) return ESC;
                if(read(fd, seq+1, 1) == 0) return ESC;

                // ESC [ sequences
                if(seq[0] == '[') {
                    if(seq[1] >= '0' && seq[1] <= '9') {
                        // extended escape so read additional byte
                        if(read(fd, seq+2, 1) == 0) return ESC;
                        if(seq[2] == '~') {
                            switch(seq[1]) {
                                case '3': return DEL_KEY;
                                case '5': return PAGE_UP;
                                case '6': return PAGE_DOWN;
                            }
                        }
                    } else {
                        switch(seq[1]) {
                            case 'A': return ARROW_UP;
                            case 'B': return ARROW_DOWN;
                            case 'C': return ARROW_RIGHT;
                            case 'D': return ARROW_LEFT;
                            case 'H': return HOME_KEY;
                            case 'F': return END_KEY;
                        }
                    }
                }

                    // ESC 0 sequences
                else if(seq[0] == 'O') {
                    switch (seq[1]) {
                        case 'H': return HOME_KEY;
                        case 'F': return END_KEY;
                    }
                }
                break;

            default:
                return c;
        }
    }
} // editor_read_key ends



/* Use the ESC [6n escape sequence to query the horizontal cursor position
 *  and return it. On error -1 is returned, on success the position of the
 *  cursor is stored at *rows and *cols and 0 is returned.
 */
int get_cursor_pos(int ifd, int ofd, int *rows, int *columns)
{
    char buf[32];
    unsigned int i = 0;

    // report cursor location
    if (write(ofd, "\x1b[6n", 4) != 4) return -1;

    // read the response: ESC [ rows; columns R
    while(i < sizeof(buf)-1) {
        if( read(ifd, buf+i, 1) != 1) break;
        if(buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    // parse it
    if (buf[0] != ESC || buf[1] != '[') return -1;
    if (sscanf(buf+2,"%d;%d",rows,columns) != 2) return -1;
    return 0;

} // get_cursor_pos ends


/* try to get the number of columns in the window */
int get_window_size(int ifd, int ofd, int *rows, int *columns)
{
    struct winsize ws;

    if(ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        // ioctl() failed so query the terminal itself
        int original_row, original_column, retval;

        // get the initail position to store for later
        retval = get_cursor_pos(ifd, ofd, &original_row, &original_column);
        if(retval == -1) goto failed;

        // go to the right bottom margin and get position
        if(write(ofd, "\x1b[999C\x1b[999B", 12) != 12) goto failed;
        retval = get_cursor_pos(ifd, ofd, rows, columns);
        if(retval == -1) goto failed;

        // restore the cursor position
        char seq[32];
        snprintf(seq, 32, "\x1b[%d;%dH", original_row, original_column);
        if(write(ofd, seq, strlen(seq)) == -1) {
            // cant recover the cursor pos ....
        }
        return 0;
    } else {
        *columns = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
    failed:
    return -1;
} //get_window_size ends

