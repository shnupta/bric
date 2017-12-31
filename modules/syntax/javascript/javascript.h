#ifndef _SYNTAX_JAVASCRIPT_H
#define _SYNTAX_JAVASCRIPT_H

// JAVASCRIPT
extern char *JAVASCRIPT_extensions[];
extern char *JAVASCRIPT_keywords[];

#define JAVASCRIPT_syntax { \
	JAVASCRIPT_extensions, \
	JAVASCRIPT_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif


