#ifndef _SYNTAX_MAKEFILE_H
#define _SYNTAX_MAKEFILE_H

// MAKEFILE
char *MAKEFILE_extensions[] = {"Makefile", NULL};
char *MAKEFILE_keywords[] = {

	// Keyword: Misc
	"$(CC)@", "CC@", "$(CFLAGS)@", "CFLAGS@", "$(MAKE)@",

	// Keyword: Stuff
	".PHONY:", "export",

	// Keyword: Basic Rules
	"all:~", "clean:~", "install:~",

	NULL
};

#define MAKEFILE_syntax { \
	MAKEFILE_extensions, \
	MAKEFILE_keywords, \
	"#", \
	"#", \
	"#", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
