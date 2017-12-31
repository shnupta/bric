#ifndef _SYNTAX_BASH_H
#define _SYNTAX_BASH_H

// BASH
extern char *BASH_extensions[];
extern char *BASH_keywords[];

#define BASH_syntax {	\
	BASH_extensions,	\
	BASH_keywords, 		\
	"#",				\
	"#", 				\
	"#", 				\
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
