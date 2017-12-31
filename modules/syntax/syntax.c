//
// Created by supreets51 on 10/12/17.
//
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
#include <stdlib.h>

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
        MAKEFILE_syntax
};

size_t __H_DB_ENTRIES = (sizeof(highlight_db)/sizeof(highlight_db[0]));