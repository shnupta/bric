//
// Created by supreets51 on 10/12/17.
//
#include <stdlib.h>
char *CSHARP_extensions[] = {".cs",NULL};
char *CSHARP_keywords[] = {
        //types and misc
        "char", "bool", "short", "int", "byte", "null",
        "long", "float", "double", "decimal", "string", "sbyte", "uint", "ulong",
        "ushort", "true", "false", "continue", "break", "enum", "struct","class",
        "namespace", "base", "new", "as", "is", "sizeof", "typeof", "stackalloc",
        "params", "ref", "out", "object", "using",
        //preprocessor
        "#define|", "#elif|", "#else|", "#error|", "#if|", "#warning|", "#undef|",
        "#line|", "#region|", "#endregion|", "#endif|",
        //conditionals
        "if~", "else~", "switch~", "case~", "try~", "throw~", "catch~", "default~",
        "finally~",
        //return
        "return#", "goto#", "in#",
        //adapters
        "const^", "static^", "public^", "protected^", "void^", "typedef^", "explicit^",
        "implicit^", "operator^", "fixed^", "lock^", "checked^", "unchecked^", "union^",
        "virtual^", "volatile^", "private^", "abstract^", "this^", "interface^",
        "override^", "readonly^", "sealed^", "unsafe^", "event^", "extern^",
        "internal^", "delegate^",
        //loops,
        "for@", "while@", "do@", "foreach@",
        NULL
};
