/* Syntax highlighting */
#ifndef _SYNTHIGH_H

#define _SYNTHIGH_H

#include "defs.h"                          /* In case this isnt already included */
#include "../modules/syntax/syntax.h"   /* For macro HL_MLCOMMENT and other HL_* macros */
#include <string.h>                     /* strchr() */
#include <ctype.h>                      /* for isspace() */
#include <stdlib.h>                     /* realloc() */

int is_separator(int c);

int editing_row_has_open_comment(editing_row *row);

void editor_update_syntax(editing_row *row);

int editor_syntax_to_colour(int highlight); //maps the syntax highlight to the terminal colours

void editor_select_syntax_highlight(char *filename); // select the correct highlight scheme based on filetype

#endif /* _SYNTHIGH_H */