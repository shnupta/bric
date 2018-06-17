#ifndef _SYNTAX_PYTHON_H
#define _SYNTAX_PYTHON_H

char *Python_extensions[] = {".py", NULL};
char *Python_keywords[] = {
	// data types
	"str", "list", "tuple", "dict", "True", "False", "int", "None", "float",
	// imports etc.
	"import|", "from|",
	//conditionals
	"if~", "elif~", "continue~", "break~", "pass~", "try~", 
	// return
	"return#", "raise#", "is#", "in#",
	// loops
	"for@", "while@", "try@", "except@", "finally@",
	// misc
	"and^", "or^", "global^", "print^", "with^", "yield^", "not^", "class^", "as^",
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
