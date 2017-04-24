#ifndef _SYNTAX_PYTHON_H
#define _SYNTAX_PYTHON_H

char *Python_extensions[] = {".py", NULL};
char *Python_keywords[] = {
	"and", "del", "from", "not", "while", "as", "elif", "or", "with", "assert", "yield", "import", "print", "exec", "in", "raise", "finally", "is", "return", "def", "for", "lambda",
	"global|", "if|", "else|", "elif|", "global|", "class|", "break|", "continue|",
	NULL
};

#define Python_syntax { \
	Python_extensions, \
	Python_keywords, \
	"#", \
	"'''", \
	"'''", \
	HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS \
}

#endif
