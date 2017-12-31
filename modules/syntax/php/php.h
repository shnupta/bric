#ifndef _SYNTAX_PHP_H
#define _SYNTAX_PHP_H
#include "../editor_syntax.h"

extern char *PHP_extensions[];
extern char *PHP_keywords[];

#define PHP_syntax { \
        PHP_extensions, \
        PHP_keywords, \
        "//", \
        "/*", \
        "*/", \
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
