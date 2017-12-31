#ifndef _LLTH_H

#define _LLTH_H

void disable_raw_mode(int fd);

void editor_at_exit(void); // called at exit to avoid remaining in raw mode

int enable_raw_mode(int fd); // lets us use terminal in raw mode

int editor_read_key(int fd); // read a key from the terminal put in raw mode

int get_cursor_pos(int ifd, int ofd, int *rows, int *columns); //using the ESC [6n escape sequence to query the cursor position and return it in *rows and *columns.

int get_window_size(int ifd, int ofd, int *rows, int *columns); // try to get the number of columns in the current terminal

#endif /* _LLTH_H */

