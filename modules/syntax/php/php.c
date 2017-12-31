//
// Created by supreets51 on 10/12/17.
//
#include <stdlib.h>
char *PHP_extensions[] = {".php",NULL};
char *PHP_keywords[] = {
        //misc
        "<?php","?>","break","class","continue","enddeclare","endfor","endforeach","endif","endswitch","endwhile","function",
        "interface","trait","use",
        //adapters
        "abstract^","callable^","const^","extends^","final^","global^","implements^","namespace^","new^","private^",
        "protected^","static^",
        //conditionals
        "if~","else~","switch~","case~","try~","catch~","finally~","and","elseif~","default#","or","throw~",
        //conditionals
        "if~","else~","switch~","case~","try~","catch~","finally~",
        //return
        "return#","goto#",
        //loops
        "for@","do@","while@","foreach@",
        //special
        "__halt_compiler~","array~","as","__clone~","declare~","die~","echo~","empty~","eval~","exit~","include~","list~",
        "include_once~","instanceof","isset~","print~","require~","require_once~","unset~",
        NULL
};
