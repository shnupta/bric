#ifndef _SYNTAX_RUST_H
#define _SYNTAX_RUST_H

//RUST
extern char *RUST_extensions[];
extern char *RUST_keywords[];

#define RUST_syntax { \
	RUST_extensions, \
	RUST_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
