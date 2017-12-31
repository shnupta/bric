//
// Created by supreets51 on 10/12/17.
//
#include <stdlib.h>
char *JAVA_extensions[] = {".java", NULL};
char *JAVA_keywords[] = {
        //types and misc
        "boolean", "double", "byte", "int", "short", "char", "long", "float",
        "continue", "break", "class", "enum", "import", "package", "super",

        //conditionals
        "switch~", "if~", "throw~", "else~", "throws~", "case~", "try~", "catch~",
        "finally~", "default~", "assert~", "instanceof~",

        //return
        "return#", "goto#",

        //adapters
        "const^", "abstract^", "new^", "public^", "protected^", "private^", "final^",
        "this^", "static^", "void^", "volatile^", "interface^", "implements^",
        "extends^", "synchronized^", "native^", "strictfp^", "transient^",

        //loops
        "for@", "while@", "do@",

        NULL
};