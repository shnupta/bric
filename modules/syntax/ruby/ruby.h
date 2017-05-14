#ifndef _SYNTAX_RUBY_H
#define _SYNTAX_RUBY_H

char *RUBY_extensions[] = {".rb", NULL};
char *RUBY_keywords[] = {
	// data types
	"true", "false", "nil",
	//properties of the current source file.
	"__ENCODING__|", "__LINE__|",  "__FILE__|",
	//conditionals
	"if~", "else~", "elsif~", "case~", "break~", "ensure~", "begin~", "rescue~",
	"then~", "when~",
	// return
	"return#", "in#", "self#", "defined?^",
	// loops
	"for@", "do@", "while@", "until@",
	// misc
	"and^", "or^", "yield^", "not^", "class^", "BEGIN^", "END^", "alias^",
	"redo^", "module^", "super^", "def^", "end^", "retry^", "undef^",
	"unless^", "next^",
	NULL
};

#define RUBY_syntax { \
	RUBY_extensions, \
	RUBY_keywords, \
	"#", \
	"=begin", \
	"=end", \
	HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS \
}

#endif
