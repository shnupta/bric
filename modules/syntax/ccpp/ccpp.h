#ifndef _SYNTAX_CCPP_H
#define _SYNTAX_CCPP_H

// C and C++
char *CCPP_extensions[] = {".c", ".cpp", ".h" ,NULL};
char *CCPP_keywords[] = {
       "asm", "auto", "break", "case", "catch", "const", "const_cast", "continue", "default", "delete", "do", "dynamic_cast", "else", "explicit", "export", "extern", "false", "for", "friend", "goto", "if", "mutable", "new", "operator", "private", "public", "register", "reinterpret_cast", "return", "sizeof", "static", "static_cast", "switch", "template", "throw", "try", "typedef", "typeid", "typename", "using", "virtual", "volatile", "while", 
	//types        
	"short|", "bool|", "true|", "false|", "void|", "wchar_t|", "struct|", "union|", "enum|" ,"int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
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
