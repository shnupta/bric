#ifndef _SYNTAX_SQL_H
#define _SYNTAX_SQL_H

// MySQL, just the beginning
extern char *SQL_extensions[];
extern char *SQL_keywords[];

#define SQL_syntax { \
	SQL_extensions, \
	SQL_keywords, \
	"-- ", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
