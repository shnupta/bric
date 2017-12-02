#ifndef _SYNTAX_STANDARD_H
#define _SYNTAX_STANDARD_H

#include "editor_syntax.h"
#include "ccpp/ccpp.h"
#include "csharp/csharp.h"
#include "python/python.h"
#include "php/php.h"
#include "pascal/pascal.h"
#include "sql/sql.h"
#include "java/java.h"
#include "javascript/javascript.h"
#include "go/go.h"
#include "ruby/ruby.h"
#include "rust/rust.h"
#include "html/html.h"
#include "d/d.h"
#include "brain/brain.h"
#include "bash/bash.h"
#include "makefile/makefile.h"
#include "dockerfile/dockerfile.h"
#include "swift/swift.h"

// Syntax highlighting macros
#define HL_NORMAL 0
#define HL_NONPRINT 1
#define HL_COMMENT 2
#define HL_MLCOMMENT 3
#define HL_STRING 4
#define HL_NUMBER 5
#define HL_MATCH 6 //a search match
#define HL_KEYWORD_TYPE 10
#define HL_KEYWORD_PP 11
#define HL_KEYWORD_COND 12
#define HL_KEYWORD_RETURN 13
#define HL_KEYWORD_ADAPTER 14
#define HL_KEYWORD_LOOP 15
#define HL_BACKGROUND_DEFAULT 16

#define HL_HIGHLIGHT_STRINGS (1<<0)
#define HL_HIGHLIGHT_NUMBERS (1<<1)


typedef struct hlcolour {
        int r, g, b;
} hlcolour;

// here is an array of syntax highlights by extensions, keywords, comments, del
struct editor_syntax highlight_db[] = {
  CCPP_syntax,
  CSHARP_syntax,
  Python_syntax,
  PHP_syntax,
  PAS_syntax,
  SQL_syntax,
  JAVA_syntax,
  JAVASCRIPT_syntax,
  GO_syntax,
  RUBY_syntax,
  RUST_syntax,
  HTML_syntax,
  D_syntax,
  BRAIN_syntax,
  BASH_syntax,
  MAKEFILE_syntax,
  DOCKERFILE_syntax,
  SWIFT_syntax
};

#define HIGHLIGHT_DB_ENTRIES (sizeof(highlight_db)/sizeof(highlight_db[0]))


#endif
