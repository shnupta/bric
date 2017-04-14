#ifndef _SYNTAX_CCPP_H
#define _SYNTAX_CCPP_H

// C and C++
char *CCPP_extensions[] = {".c", ".cpp", ".h" ,NULL};
char *CCPP_keywords[] = {
	//types and misc
	"char", "bool", "short", "int", "__int8", "__int16", "__int32", "__int64",
	"long", "wchar_t", "__wchar_t", "float", "double", "true", "false",
	"continue", "break", "enum", "struct",
	//preprocessor
	"#define|", "#elif|", "#else|", "#ifndef|", "#error|", "#if|", "#ifdef|",
	"#pragma|", "#import|", "#include|", "#line|", "#undef|", "#using|", "#endif|",
	//conditionals
	"if~", "else~", "switch~", "case~", "try~", "throw~", "catch~",
	//return
	"return#", "goto#",
	//adapters
	"const`", "static`", "public`", "protected`", "void`", "typedef`", 
	"union`", "virtual`", "volatile`", "private`",  
	//loops,
	"for@", "while@", "do@",
	NULL
};


//struct editor_syntax CCPP = {
//	CCPP_extensions,
//	CCPP_keywords,
//	"//",
//	"/*",
//	"*/",
//	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
//};


#define CCPP_syntax { \
	CCPP_extensions, \
	CCPP_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}




#endif
