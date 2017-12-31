#ifndef _SYNTAX_D_H
#define _SYNTAX_D_H

// D Lang
extern char *D_extensions[];
extern char *D_keywords[];

#define D_syntax { \
	D_extensions, \
	D_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}




#endif
