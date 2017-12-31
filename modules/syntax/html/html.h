#ifndef _SYNTAX_HTML_H
#define _SYNTAX_HTML_H

//html
extern char *HTML_extensions[];
extern char *HTML_keywords[];

#define HTML_syntax { \
	HTML_extensions, \
	HTML_keywords, \
	"//", \
	"<!--", \
	"-->", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
