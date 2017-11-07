#ifndef _SYNTAX_JAVASCRIPT_H
#define _SYNTAX_JAVASCRIPT_H

// JAVASCRIPT
char *JAVASCRIPT_extensions[] = {".js",NULL};
char *JAVASCRIPT_keywords[] = {
	//types and misc
	"boolean", "double", "byte", "int", "short", "char", "long", "float","var","null","true",
	"continue", "break", "class", "enum", "import","export","package", "super","false","let",
	"function","with","default","yield","delete",

	//conditionals
	"switch~", "if~", "throw~", "else~", "throws~", "case~", "try~", "catch~",
	"finally~", "default~", "instanceof~","typeof~",

	//return
	"return#", "goto#","in#",

	//adapters
	"const^", "abstract^", "new^", "public^", "protected^", "private^", "final^",
	"this^", "static^", "void^", "volatile^", "interface^", "implements^",
	"extends^", "synchronized^", "native^","transient^","const^","debugger^",

	//loops
	"for@", "while@", "do@","each@",

	NULL
};

#define JAVASCRIPT_syntax { \
	JAVASCRIPT_extensions, \
	JAVASCRIPT_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif


