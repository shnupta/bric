#ifndef _SYNTAX_SQL_H
#define _SYNTAX_SQL_H

// MySQL, just the beginning
char *SQL_extensions[] = {".sql" ,NULL};
char *SQL_keywords[] = {
	"create","insert","from","values","select","alter","orderby",
	"table~","database~","column~",
	"asc@","desc@",
	NULL
};

#define SQL_syntax { \
	SQL_extensions, \
	SQL_keywords, \
	"-- ", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
