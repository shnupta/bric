// find mode and replace mode!
// Created by supreets51 on 10/12/17.
//

#ifndef BRIC_FINDMODE_H
#define BRIC_FINDMODE_H

#define BRIC_QUERY_LENGTH 256

void editor_find(int fd);

void editor_find_replace(int fd);

void editor_goto(int linenumber);

#endif //BRIC_FINDMODE_H
