#ifndef EDITOR_H
#define EDITOR_H

#include <time.h>

#define CTRL_KEY(k) ((k)&0x1f)

/* Editing ROW */
typedef struct editing_row
{
  int index;                    // the index of the row in the file
  int size;                     // size of the row (not including null terminator)
  int rendered_size;            // size of the rendered row
  char *chars;                  // the content of the row
  char *rendered_chars;         // the content of the rendered row
  unsigned char *hl;            // syntax highlighting for each character in the rendered row
  int hl_open_comment;          // the row had an open comment
  struct editing_row *prev;
  struct editing_row *next;
} editing_row;

/* Colour MAP */
typedef struct colour_map
{
  int hl_comment_colour;
  int hl_mlcomment_colour;
  int hl_keyword_cond_colour;
  int hl_keyword_type_colour;
  int hl_keyword_pp_colour;
  int hl_keyword_return_colour;
  int hl_keyword_adapter_colour;
  int hl_keyword_loop_colour;
  int hl_string_colour;
  int hl_number_colour;
  int hl_match_colour;
  int hl_background_colour;
  int hl_default_colour;
} colour_map;

/* Config EDITOR */
struct editor_config
{
  int cursor_x, cursor_y;       // the x and y of the cursor
  int row_offset;               // offset of the displayed row
  int column_offset;            // offset of the displayed column
  int screen_rows;              // number of rows that can be shown
  int screen_columns;           // number of columns that can be shown
  int num_of_rows;              // number of rows
  int rawmode;                  // is terminal raw mode enabled?
  //editing_row *row;               // the rows
  editing_row *row_head;
  editing_row *row_tail;
  editing_row *current;
  int dirty;                    // if the file is modified but not saved
  int yank_buffer_len;          // length of yank buffer
  char *yank_buffer;            // buffer to hold yanked/copied text
  int newfile;                  // is currently opened a new file?
  char *filename;               // currently open filename
  char status_message[256];
  time_t status_message_time;
  struct editor_syntax *syntax; // current syntax highlighting
  int line_numbers;             // show line numbers
  int indent;                   // tabs and spaces indentation
  int tab_length;               //number of spaces when tab pressed
  colour_map colours;           // highlight colours
  int mode;                     // selection or normal mode
  int selected_base_x;
  int selected_base_y;
  char *clipboard;
  char prev_char;
};

/* Key ACTION */
enum KEY_ACTION
{
  KEY_NULL = 0,
  CTRL_G = CTRL_KEY ('g'),
  CTRL_R = CTRL_KEY ('r'),
  CTRL_Y = CTRL_KEY ('y'),
  CTRL_P = CTRL_KEY ('p'),
  CTRL_M = CTRL_KEY ('m'),
  CTRL_N = CTRL_KEY ('n'),
  CTRL_A = 1,
  CTRL_C = 3,
  CTRL_D = 4,
  CTRL_F = 6,
  CTRL_H = 8,
  TAB = 9,
  CTRL_L = 12,
  ENTER = 13,
  CTRL_Q = 17,
  CTRL_S = 19,
  CTRL_U = 21,
  CTRL_V = 22,
  ESC = 27,
  BACKSPACE = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};

#endif
