#ifndef _SYNTAX_BASH_H
#define _SYNTAX_BASH_H

// BASH
char *BASH_extensions[] = {".sh", NULL};
char *BASH_keywords[] = {

	// Keyword: Misc	
	"function", "select", "read", "echo", "break", 
	"export", "let", "clear", "exit",
	"!", ";;", "|", ">", ">>", "<<", "<", "&",

	// Keyword: Conditionals
	"if~", "elif~", "else~", "fi~",  "case~", "esac~", "then~",

	// Keyword: Return
	"return#",

	// Keyword: Loops
	"for@", "while@", "until@", "in@", "do@", "done@",

	NULL
};

#define BASH_syntax {	\
	BASH_extensions,	\
	BASH_keywords, 		\
	"#",				\
	"#", 				\
	"#", 				\
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
