//
// Created by supreets51 on 10/12/17.
//
#include <stdlib.h>
char *MAKEFILE_extensions[] = {"Makefile", NULL};
char *MAKEFILE_keywords[] = {

        // Keyword: Misc
        "$(CC)@", "CC@", "$(CFLAGS)@", "CFLAGS@", "$(MAKE)@",

        // Keyword: Stuff
        ".PHONY:", "export",

        // Keyword: Basic Rules
        "all:~", "clean:~", "install:~",

        NULL
};
