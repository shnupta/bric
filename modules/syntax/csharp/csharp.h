#ifndef _SYNTAX_CSHARP_H
#define _SYNTAX_CSHARP_H

// C#
extern char *CSHARP_extensions[];
extern char *CSHARP_keywords[];


#define CSHARP_syntax { \
	CSHARP_extensions, \
	CSHARP_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
