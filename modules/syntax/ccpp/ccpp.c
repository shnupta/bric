#include <stdlib.h>

char *CCPP_extensions[] = {".c", ".cpp", ".h" , ".hpp", NULL};
char *CCPP_keywords[] = {
        //types and misc
        "char", "bool", "short", "int", "__int8", "__int16", "__int32", "__int64",
        "long", "wchar_t", "__wchar_t", "float", "double", "true", "false",
        "continue", "break", "enum", "struct","class","default#","namespace",
        //preprocessor
        "#define|", "#elif|", "#else|", "#ifndef|", "#error|", "#if|", "#ifdef|",
        "#pragma|", "#import|", "#include|", "#line|", "#undef|", "#using|", "#endif|",
        //conditionals
        "if~", "else~", "switch~", "case~", "try~", "throw~", "catch~",
        //return
        "return#", "goto#",
        //adapters
        "const^", "static^", "public^", "protected^", "void^", "typedef^",
        "union^", "virtual^", "volatile^", "private^", "register^",
        //loops,
        "for@", "while@", "do@",
        NULL
};