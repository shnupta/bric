/**
 * Created by frags51 on 9/12/17.
 * Contains implementations of functions for syntax highlightuing
 * WARNING: In manny places, size_t has been implicitly converted to int. Can it lead to an error?
 */

#include "synthigh.h"

extern struct editor_config Editor; /* Defined in bric.c, used in: editor_syntax_to_colour */

int is_separator(int c)
{
    return c == '\0' || isspace(c) || strchr(",.()+_/*=%[];", c) != NULL;
}

/* Return true if the specified row's last char is part of
 * a multiline comment that spans to the next row
 */
int editing_row_has_open_comment(editing_row *row)
{
    if(row->hl && row->rendered_size && row->hl[row->rendered_size-1] == HL_MLCOMMENT && (row->rendered_size < 2 || (row->rendered_chars[row->rendered_size-2] != '*' || row->rendered_chars[row->rendered_size-1] != '/'))) return 1;

    return 0;
}

// set every byte of row->hl (every character in the line) to the right syntax type defined by HL_*
void editor_update_syntax(editing_row *row)
{
    row->hl = realloc(row->hl, row->rendered_size);
    memset(row->hl, HL_NORMAL, row->rendered_size);

    if(Editor.syntax == NULL) return; //no syntax in this line, everything is HL_NORMAL

    int i, prev_sep, in_string, in_comment;
    char *p;
    char **keywords = Editor.syntax->keywords;
    char *scs = Editor.syntax->singleline_comment_start;
    char *mcs = Editor.syntax->multiline_comment_start;
    char *mce = Editor.syntax->multiline_comment_end;

    // point to the first non space char
    p = row->rendered_chars;
    i = 0; // current char offset in row
    while (*p && isspace(*p)) {
        p++;
        i++;
    }
    prev_sep = 1; // tell the parser if 'i' points to the start of a word
    in_string = 0; // is the character  in a string "" or ''
    in_comment = 0; // is the char in an open multiline comment

    //if the previous line has open open comment then this line start with an open comment state
    if(row->index > 0 && editing_row_has_open_comment(row->prev)) in_comment = 1;
    while (*p != '\0') {
        //handle single line comments
        if(prev_sep && *p == scs[0] && *(p+1) == scs[1]) {
            // from here to the end of the row is a comment
            memset(row->hl+i, HL_COMMENT, row->rendered_size-i);
            return;
        }

        // handle multiline comments
        if(in_comment) {
            row->hl[i] = HL_MLCOMMENT;
            if(*p == mce[0] && *(p+1) == mce[1]) {
                row->hl[i+1] = HL_MLCOMMENT;
                p += 2; i+= 2;
                in_comment = 0;
                prev_sep = 1;
                continue;
            } else {
                prev_sep = 0;
                p++; i++;
                continue;
            }
        } else if(*p == mcs[0] && *(p+1) == mcs[1]) {
            row->hl[i] = HL_MLCOMMENT;
            row->hl[i+1] = HL_MLCOMMENT;
            p += 2; i += 2;
            in_comment = 1;
            prev_sep = 0;
            continue;
        }


        // handle "" and ''
        if(in_string) {
            row->hl[i] = HL_STRING;
            if(*p == '\\') {
                row->hl[i+1] = HL_STRING;
                p += 2; i += 2;
                prev_sep = 0;
                continue;
            }
            if(*p == in_string) in_string = 0;
            p++; i++;
            continue;
        } else {
            if(*p == '"' || *p == '\'') {
                in_string = *p;
                row->hl[i] = HL_STRING;
                p++; i++;
                prev_sep = 0;
                continue;
            }
        }

        // handle non printable chars
        if(!isprint(*p)) {
            row->hl[i] = HL_NONPRINT;
            p++; i++;
            prev_sep = 0;
            continue;
        }

        //handle numbers
        if((isdigit(*p) && (prev_sep || row->hl[i-1] == HL_NUMBER)) || (*p == '.' && i > 0 && row->hl[i-1] == HL_NUMBER)) {
            row->hl[i] = HL_NUMBER;
            p++; i++;
            prev_sep = 0;
            continue;
        }

        //handle keywords and lib calls
        if(prev_sep) {
            int j;
            for(j = 0; keywords[j]; j++) {
                int klen = strlen(keywords[j]);
                int pp = keywords[j][klen-1] == '|'; // preprocessor keyword
                int cond = keywords[j][klen-1] == '~'; // condition
                int retu = keywords[j][klen-1] == '#';
                int adapter = keywords[j][klen-1] == '^'; //adapter keywords
                int loopy = keywords[j][klen-1] == '@';
                if(pp || cond || retu || adapter || loopy) klen--;

                if(!memcmp(p, keywords[j], klen) && is_separator(*(p+klen))) {
                    // keyword
                    if(pp) memset(row->hl+i, HL_KEYWORD_PP, klen);
                    else if(cond) memset(row->hl+i, HL_KEYWORD_COND, klen);
                    else if(retu) memset(row->hl+i, HL_KEYWORD_RETURN, klen);
                    else if(adapter) memset(row->hl+i, HL_KEYWORD_ADAPTER, klen);
                    else if(loopy) memset(row->hl+i, HL_KEYWORD_LOOP, klen);
                    else memset(row->hl+i, HL_KEYWORD_TYPE, klen);
                    p += klen;
                    i += klen;
                    break;
                }
            }
            if(keywords[j] != NULL) {
                prev_sep = 0;
                continue; // we had a keyword match
            }
        }

        // not special chars
        prev_sep = is_separator(*p);
        p++; i++;
    }

    // propagate a syntax change to the next row if the open comment state is changed
    int oc = editing_row_has_open_comment(row);
    if(row->hl_open_comment != oc && row->index+1 < Editor.num_of_rows) editor_update_syntax(row->next);
    row->hl_open_comment = oc;
} // editor_update_syntax ends


// map thee syntax highlight types to terminal colours
int editor_syntax_to_colour(int highlight)
{
    switch(highlight) {
        case HL_COMMENT: return Editor.colours.hl_comment_colour;
        case HL_MLCOMMENT: return Editor.colours.hl_mlcomment_colour;
        case HL_KEYWORD_COND: return Editor.colours.hl_keyword_cond_colour;
        case HL_KEYWORD_TYPE: return Editor.colours.hl_keyword_type_colour;
        case HL_KEYWORD_PP: return Editor.colours.hl_keyword_pp_colour;
        case HL_KEYWORD_RETURN: return Editor.colours.hl_keyword_return_colour;
        case HL_KEYWORD_ADAPTER: return Editor.colours.hl_keyword_adapter_colour;
        case HL_KEYWORD_LOOP: return Editor.colours.hl_keyword_loop_colour;
        case HL_STRING: return Editor.colours.hl_string_colour;
        case HL_NUMBER: return Editor.colours.hl_number_colour;
        case HL_MATCH: return Editor.colours.hl_match_colour;
        case HL_BACKGROUND_DEFAULT: return Editor.colours.hl_background_colour;
        default: return Editor.colours.hl_default_colour;
    }
} // editor_syntax_to_colour ends


// select the syntax highlight scheme depending on the filename
void editor_select_syntax_highlight(char *filename)
{
    for(unsigned int j = 0; j < HIGHLIGHT_DB_ENTRIES; j++) {
        struct editor_syntax *s = highlight_db+j;
        unsigned int i = 0;
        while(s->filematch[i]) {
            char *p;
            int patlen = strlen(s->filematch[i]);
            if((p = strstr(filename, s->filematch[i])) != NULL) {
                if(s->filematch[i][0] != '.' || p[patlen] == '\0') {
                    Editor.syntax = s;
                    return;
                }
            }
            i++;
        }
    }
} //editor_select_syntax_highlight ends


