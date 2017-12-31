//
// Created by supreets51 on 10/12/17.
//
#include <stdlib.h>
char *RUBY_extensions[] = {".rb", NULL};
char *RUBY_keywords[] = {
        // data types
        "true", "false", "nil",
        //properties of the current source file.
        "__ENCODING__|", "__LINE__|",  "__FILE__|",
        //conditionals
        "if~", "else~", "elsif~", "case~", "break~", "ensure~", "begin~", "rescue~",
        "then~", "when~",
        // return
        "return#", "in#", "self#", "defined?^",
        // loops
        "for@", "do@", "while@", "until@",
        // misc
        "and^", "or^", "yield^", "not^", "class^", "BEGIN^", "END^", "alias^",
        "redo^", "module^", "super^", "def^", "end^", "retry^", "undef^",
        "unless^", "next^",
        NULL
};