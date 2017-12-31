#ifndef _SYNTAX_BRAIN_H
#define _SYNTAX_BRAIN_H

extern char *BRAIN_extensions[];
extern char *BRAIN_keywords[];

// Anything else than those keywords are considered comments in BRAIN.

#define BRAIN_syntax {       \
        BRAIN_extensions,    \
        BRAIN_keywords,      \
        "",                  \
        "",                  \
	"",                  \
        HL_HIGHLIGHT_STRINGS \
}

#endif // _SYNTAX_BRAIN_H
