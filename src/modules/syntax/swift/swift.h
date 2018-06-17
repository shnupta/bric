#ifndef _SYNTAX_SWIFT_H
#define _SYNTAX_SWIFT_H

// SWIFT
char *SWIFT_extensions[] = {".swift", NULL};
char *SWIFT_keywords[] = {

    // Keyword: Declaration
    "class#",
    "deinit#",
    "enum#",
    "extension#",
    "func#",
    "import#",
    "init#",
    "internal#",
    "let#",
    "operator#",
    "private#",
    "protocol#",
    "public#",
    "static#",
    "struct#",
    "subscript#",
    "typealias#",
    "var#",


    // Keyword: Statement
    "break~",
    "case~",
    "continue~",
    "default~",
    "do~",
    "else~",
    "fallthrough~",
    "for~",
    "if~",
    "in~",
    "return~",
    "switch~",
    "where~",
    "while~",


    // Keyword: Expression
    "as@",
    "dynamicType@",
    "false@",
    "is@",
    "nil@",
    "self@",
    "Self@",
    "super@",
    "true@",
    "__COLUMN__@",
    "__FILE__@",
    "__FUNCTION__@",
    "__LINE__@",


    // Keyword: Special
    "associativity^",
    "convenience^",
    "dynamic^",
    "didSet^",
    "final^",
    "get^",
    "infix^",
    "inout^",
    "lazy^",
    "left^",
    "mutating^",
    "none^",
    "nonmutating^",
    "optional^",
    "override^",
    "postfix^",
    "precedence^",
    "prefix^",
    "Protocol^",
    "required^",
    "right^",
    "set^",
    "Type^",
    "unowned^",
    "weak^",

    NULL
};

#define SWIFT_syntax { \
        SWIFT_extensions, \
        SWIFT_keywords,   \
        "//",    \
        "/*",     \
        "*/",     \
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
