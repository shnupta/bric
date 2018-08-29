#ifndef _SYNTAX_HAXE_H
#define _SYNTAX_HAXE_H

// HAXE
char *HAXE_extensions[] = {".hx",NULL};
char *HAXE_keywords[] = {
	//types and misc
	"Bool", "Float", "Int", "true", "false", "String",
	"null", "Void", "NaN", "Array",
	"var", "trace", "Node", "TString", "TInt", "Class",
	"EReg",

	//conditionals
	"switch~", "if~", "throw~", "else~", "case~", "try~", "catch~",
	"Type~",

	//return
	"return#", "break#","continue#",

	//adapters
	"static^", "public^", "protected^", "private^", "class^", "extern^",
	"function^", "import^", "using^", "typedef^", "inline^", "package^",
	"cast^", "implements^",

	//loops
	"for@", "while@", "do@", "in@", "...@",

	// Should figure out metadata like @:keep and @:rtti selectors in haxe.
	// https://haxe.org/manual/lf-metadata.html

	NULL
};

#define HAXE_syntax { \
	HAXE_extensions, \
	HAXE_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
