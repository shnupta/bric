#ifndef _SYNTAX_BRAIN_H
#define _SYNTAX_BRAIN_H


char *brain_extensions[] = {".brain", ".b", NULL};
char *brain_keywords[] = {
    "+", "-", "#", ".", ",", ">", "<", "*", "[", "]", "{", "}",
    "$", "/", "%", "!", "?", ":", ";", "^"
};

// Anything else than those keywords are considered comments in Brain.

#define brain_syntax {      \
        brain_extensions,   \
        brain_keywords      \
}


#endif // _SYNTAX_BRAIN_H
