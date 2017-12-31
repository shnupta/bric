//
// Created by supreets51 on 10/12/17.
//
#include <stdlib.h>
char *JAVASCRIPT_extensions[] = {".js",NULL};
char *JAVASCRIPT_keywords[] = {
        //types and misc
        "boolean", "double", "byte", "int", "short", "char", "long", "float","var","null","true",
        "continue", "break", "class", "enum", "import","export","package", "super","false","let",
        "function","with","default","yield","delete",

        //conditionals
        "switch~", "if~", "throw~", "else~", "throws~", "case~", "try~", "catch~",
        "finally~", "default~", "instanceof~","typeof~",

        //return
        "return#", "goto#","in#",

        //adapters
        "const^", "abstract^", "new^", "public^", "protected^", "private^", "final^",
        "this^", "static^", "void^", "volatile^", "interface^", "implements^",
        "extends^", "synchronized^", "native^","transient^","const^","debugger^",

        //loops
        "for@", "while@", "do@","each@",

        NULL
};