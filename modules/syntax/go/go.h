#ifndef _SYNTAX_GO_H
#define _SYNTAX_GO_H

// GOLANG
char *GO_extensions[] = {".go",NULL};
char *GO_keywords[] = {
	//types and misc
	"bool", "byte", "int8", "float32","float64","uint64","complex128","complex64","uint32 ","var","null","true",
	, "break", "import","package","false","const","float32","float64"," rune","string","uintptr",
	"func",

	//conditionals
	"switch~", "if~", "else~","case~","default~",

	//return
	"return#", "goto#",


	//loops
	"for@",

	NULL
};

#define GO_syntax { \
	GO_extensions, \
	GO_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
