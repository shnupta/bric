#ifndef _EDITOR_SYNTAX_H
#define _EDITOR_SYNTAX_H

#define HL_HIGHLIGHT_STRINGS (1<<0)
#define HL_HIGHLIGHT_NUMBERS (1<<1)

struct editor_syntax {
        char **filematch;
        char **keywords;
        char singleline_comment_start[3];
        char multiline_comment_start[6];
        char multiline_comment_end[6];
        int flags;
};


#endif
