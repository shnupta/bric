#ifndef _SYNTAX_PASCAL_H
#define _SYNTAX_PASCAL_H

// pascal
extern char *PAS_extensions[];
extern char *PAS_keywords[];



#define PAS_syntax { \
	PAS_extensions, \
	PAS_keywords, \
    "#", \
	"{", \
	"}", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}




#endif
