#include "bric.h"

// Low level terminal handling
 void disable_raw_mode(int fd)
{
        // dont bother checking the return value as its too late
        if (editor.rawmode) {
                tcsetattr(fd, TCSAFLUSH, &orig_termios);
                editor.rawmode = 0;
        }
}


void editor_at_exit(void) 
{
        disable_raw_mode(STDIN_FILENO);
}

int enable_raw_mode(int fd) 
{
        struct termios raw;

        if(editor.rawmode) return 0; //already enabled
        if(!isatty(STDIN_FILENO)) goto fatal;
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
        editor.rawmode = 1;
        return 0;

fatal:
        errno = ENOTTY;
        return -1;
}



int editor_read_key(int fd)
{

}
