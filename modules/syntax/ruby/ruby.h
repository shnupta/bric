#ifndef _SYNTAX_RUBY_H
#define _SYNTAX_RUBY_H

extern char *RUBY_extensions[];
extern char *RUBY_keywords[];

#define RUBY_syntax { \
	RUBY_extensions, \
	RUBY_keywords, \
	"#", \
	"=begin", \
	"=end", \
	HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS \
}

#endif
