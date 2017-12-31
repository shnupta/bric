//
// Created by supreets51 on 10/12/17.
//
#include <stdlib.h>
char *PAS_extensions[] = {".pas" ,NULL};
char *PAS_keywords[] = {
        //types and misc
        "array", "begin", "asm", "constructor", "destructor", "div", "end", "false",
        "file", "function", "implementation", "interface", "label", "mod", "nil",
        "not", "object", "operator", "procedure","program","record","set","shl","shr",
        "string","true","type","unit","uses","var","xor",
        //conditionals
        "and~","break~","case~","continue~","else~","if~","or~","then~",
        //goto
        "goto#",
        //adapters
        "const^", "in^", "inline^", "of^", "on^", "packed^",
        "with^",
        //loops,
        "for@", "while@", "do@","downto@","repeat@","to@","until@",
        NULL
};
