#ifndef _SYNTAX_PYTHON_H
#define _SYNTAX_PYTHON_H

extern char *Python_extensions[];
extern char *Python_keywords[];

#define Python_syntax { \
	Python_extensions, \
	Python_keywords, \
	"#", \
	"'''", \
	"'''", \
	HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS \
}

#endif
