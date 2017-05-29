#ifndef _SYNTAX_RUST_H
#define _SYNTAX_RUST_H

//rust
char *rust_extensions[] = {".rs", ".rlib", NULL};
char *rust_keywords[] = {
  // types and misc
  "str", "bool", "char", "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "isize", "usize", "f32", "f64", "struct", "enum", "true", "false",
  // cargo stuff
  "extern|", "crate|", "use|", "mod|", "super|",
  // conditionals
  "if~", "else~", "match~", "Ok~", "Some~", "Result~",
  // loops
  "loop#", "while#", "for#", "break#", "continue#", "do#",
  // modifiers and adapters
  "mut^", "const^", "pub^", "self^", "virtual^", "Self^",
  // other misc
  "type", "typeof", "sizeof", "move", "box", "become", "macro", "final", "as", "abstract",
  "alignof", "macro", "offsetof", "override", "priv", "proc", "pure", "ref",
  "where", "unsafe", "unsized", "let", "in",
  //function stuff
  "fn@", "yield@", "trait@", "impl@",

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
