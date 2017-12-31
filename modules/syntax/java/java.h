#ifndef _SYNTAX_JAVA_H
#define _SYNTAX_JAVA_H

//Java
extern char *JAVA_extensions[];
extern char *JAVA_keywords[];

#define JAVA_syntax { \
	JAVA_extensions, \
	JAVA_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
