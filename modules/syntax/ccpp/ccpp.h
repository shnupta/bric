#ifndef _SYNTAX_CCPP_H
#define _SYNTAX_CCPP_H

// C and C++
/* Header file should consist of definitions ideally. Including this multiple times is leading to
 * multiple definition error. */
extern char *CCPP_extensions[];
extern char *CCPP_keywords[];

//struct editor_syntax CCPP = {
//	CCPP_extensions,
//	CCPP_keywords,
//	"//",
//	"/*",
//	"*/",
//	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
//};

#define CCPP_syntax { \
	CCPP_extensions, \
	CCPP_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif