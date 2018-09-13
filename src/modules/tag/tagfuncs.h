//Header file for tag movement function prototypes

#include "tagstack.h"

#define MOVE_AHEAD 101
#define MOVE_BACK 102

tagstack tag_stack;

int handle_tag_movement (int where);
char *get_key (void);
int char_check (char c);
int tagsearch (char *tosearch);
