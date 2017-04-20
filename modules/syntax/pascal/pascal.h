#ifndef _SYNTAX_PASCAL_H
#define _SYNTAX_PASCAL_H

// pascal
char *PAS_extensions[] = {".pas" ,NULL};
char *PAS_keywords[] = {
	//types and misc
	"array", "begin", "asm", "constructor", "destructor", "div", "end", "false",
	"file", "function", "implementation", "interface", "label", "mod", "nil",
	"not", "object", "operator", "procedure","program","record","set","shl","shr",
	"string","true","type","unit","uses","var","xor",
	//conditionals
	"and~","break~","case~","continue~","else~","if~","or~","then~",
	//goto
	"goto#",
	//adapters
	"const^", "in^", "inline^", "of^", "on^", "packed^", 
	"with^",
	//loops,
	"for@", "while@", "do@","downto@","repeat@","to@","until@",
	NULL
};



#define Pascal_syntax { \
	PAS_extensions, \
	PAS_keywords, \
	"{", \
	"}", \
	"//", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}




#endif
