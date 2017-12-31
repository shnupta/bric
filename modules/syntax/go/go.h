#ifndef _SYNTAX_GO_H
#define _SYNTAX_GO_H

// GOLANG
extern char *GO_extensions[];
extern char *GO_keywords[];

#define GO_syntax { \
	GO_extensions, \
	GO_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
