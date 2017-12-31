#ifndef _SYNTAX_MAKEFILE_H
#define _SYNTAX_MAKEFILE_H

// MAKEFILE
extern char *MAKEFILE_extensions[] ;
extern char *MAKEFILE_keywords[];

#define MAKEFILE_syntax { \
	MAKEFILE_extensions, \
	MAKEFILE_keywords, \
	"#", \
	"#", \
	"#", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
