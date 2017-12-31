//
// Created by supreets51 on 10/12/17.
//
#include <stdlib.h>
char *D_extensions[] = {".d",NULL};
char *D_keywords[] = {
        //types and misc
        "inout", "shared", "bool", "byte", "ubyte", "short", "ushort", "int",
        "uint", "long", "ulong", "char", "wchar", "dchar", "float", "double",
        "real", "ifloat", "idouble", "ireal", "cfloat", "cdouble", "creal",
        "typeof", "true", "false", "this", "super", "null", "is", "struct",
        "union", "class", "enum", "ref", "delegate", "function", "string",
        "__vector", "new", "delete", "cast", "mixin", "import", "__traits",
        "default", "with", "finally", "asm", "scope", "auto", "extern",
        "shared", "__gshared", "nothrow", "pure", "lazy", "in", "invariant",
        "out", "body", "unittest", "template", "alias", "pragma",
        //property identifiers
        "@property", "@safe", "@trusted", "@system", "@disable", "@nogc",
        //conditionals
        "if~", "else~", "switch~", "case~", "try~", "throw~", "catch~", "version~",
        "debug~",
        //return
        "return#", "goto#",
        //adapters
        "const^", "immutable^", "inout^", "shared^", "static^", "public^",
        "protected^", "private^", "void^", "typedef^", "union^", "virtual^",
        "volatile^", "final^", "deprecated^", "synchronized^", "override^",
        "interface^", "export^", "package^", "abstract^",
        //loops,
        "for@", "foreach@", "foreach_reserve@", "while@", "do@",
        NULL
};