#include <stdlib.h>
char *BASH_extensions[] = {".sh", NULL};
char *BASH_keywords[] = {

        // Keyword: Misc
        "function", "select", "read", "echo", "break",
        "export", "let", "clear", "exit",
        "!", ";;", "|", ">", ">>", "<<", "<", "&",

        // Keyword: Conditionals
        "if~", "elif~", "else~", "fi~",  "case~", "esac~", "then~",

        // Keyword: Return
        "return#",

        // Keyword: Loops
        "for@", "while@", "until@", "in@", "do@", "done@",

        NULL
};