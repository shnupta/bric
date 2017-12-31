//
// Created by supreets51 on 10/12/17.
//
#include <stdlib.h>
char *GO_extensions[] = {".go",NULL};
char *GO_keywords[] = {
        //types and misc
        "bool", "byte", "int8", "float32","float64","uint64","complex128","complex64","uint32 ","var","null","true"
        ,"break", "import","package","false","const","float32","float64"," rune","string","uintptr",
        "func",

        //conditionals
        "switch~", "if~", "else~","case~","default~",

        //return
        "return#", "goto#",


        //loops
        "for@",

        NULL
};
