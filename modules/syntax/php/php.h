#ifndef _SYNTAX_PHP_H
#define _SYNTAX_PHP_H
#include "../editor_syntax.h"

char *PHP_extensions[] = {".php",NULL};
char *PHP_keywords[] = {
        //start
        "<?php","?>",
        //conditionals
        "if~","else~","switch~","case~","try~","catch~","finally~",
        //return
        "return#","goto#",
        //loops
        "for@","do@","while@","foreach@",
        NULL
};

#define PHP_syntax { \
        PHP_extensions, \
        PHP_keywords, \
        "//", \
        "/*", \
        "*/", \
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
