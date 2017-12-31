//
// Created by supreets51 on 10/12/17.
//

#include <stdlib.h>
char *Python_extensions[] = {".py", NULL};
char *Python_keywords[] = {
        // data types
        "str", "list", "tuple", "dict", "True", "False", "int", "None", "float",
        // imports etc.
        "import|", "from|",
        //conditionals
        "if~", "elif~", "continue~", "break~", "pass~", "try~",
        // return
        "return#", "raise#", "is#", "in#",
        // loops
        "for@", "while@", "try@", "except@", "finally@",
        // misc
        "and^", "or^", "global^", "print^", "with^", "yield^", "not^", "class^", "as^",
        NULL
};