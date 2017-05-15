#ifndef _SYNTAX_RUST_H
#define _SYNTAX_RUST_H

//rust
char *rust_extensions[] = {".rs", ".rlib", NULL};
char *rust_keywords[] = {
  "abstract", "alignof", "as", "become", "box", "break", "const",
  "continue", "crate", "do", "else", "enum", "extern", "false", "final",
  "fn", "for", "if", "impl", "in", "let", "loop", "macro", "match", "mod",
  "move", "mut", "offsetof", "override", "priv", "proc", "pub", "pure", "ref",
  "return", "Self", "self", "sizeof", "static", "struct", "super", "trait",
  "true", "true", "type", "typeof", "unsafe", "unsized", "use", "virtual",
  "where", "while", "yield",
  NULL
};

#define rust_syntax { \
	rust_extensions, \
	rust_keywords, \
	"//", \
	"/*", \
	"*/", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
