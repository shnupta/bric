#include "bric.h"

static struct __current_file CurrentFile;
static struct editor_config Editor;
static struct termios orig_termios; // para poder restaurar o original ao sair
static int line_number_length = 3;
const char* line_number_format[] = {"%1d", "%2d", "%3d", "%4d", "%5d"};

// Captura o sinal para realizar o tratamento de acoes
void sigwinch_handler()
{
	int old_columns = Editor.screen_columns;
	if (get_window_size(STDIN_FILENO, STDOUT_FILENO, &Editor.screen_rows, &Editor.screen_columns) == -1) {
                perror("Unable to query the screen for size (columns / rows)");
                exit(1);
        }

        if (Editor.line_numbers)
        {
		Editor.screen_columns -= line_number_length;
        }
        Editor.prev_char = ' ';
        Editor.screen_rows -= 2; // aloca espaco para a barra de status

	if (Editor.cursor_y >= Editor.screen_rows)
	{
		Editor.row_offset += Editor.cursor_y - Editor.screen_rows+1;
		Editor.cursor_y = Editor.screen_rows-1;
	}

	if (Editor.cursor_x >= Editor.screen_columns)
	{
		Editor.column_offset += Editor.cursor_x - Editor.screen_columns+1;
		Editor.cursor_x = Editor.screen_columns-1;
	}
	else
	{
		int delta = Editor.screen_columns - old_columns;
		if (delta > 0 && Editor.column_offset > 0)
		{
			int change = (delta > Editor.column_offset) ? Editor.column_offset : delta;
			Editor.column_offset -= change;
			Editor.cursor_x += change;
		}
	}

	editor_refresh_screen();
}

// Tratamento do terminal a baixo nivel
void disable_raw_mode(int fd)
{   

    // nao se preocupar em checar o valor de retorno
    if (Editor.rawmode) {
        tcsetattr(fd, TCSAFLUSH, &orig_termios);
        write(fd, "\033[2J\033[H\033[?1049l", 15);
        Editor.rawmode = 0;
    }
}

// Acoes a serem realizadas ao fechar o editor
void editor_at_exit(void)
{
        disable_raw_mode(STDIN_FILENO);
}

// Habilita o modo de edicao raw (cru)
int enable_raw_mode(int fd)
{
    struct termios raw;

    if(Editor.rawmode) return 0; // ja habilitado
    if(!isatty(fd)) {
        errno = ENOTTY;
        return -1;
    }
    atexit(editor_at_exit);
    if(tcgetattr(fd, &orig_termios) == -1) {
        errno = ENOTTY;
        return -1;
    }

    raw = orig_termios; // modifica o modo original
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // output modes - disabilitar pos-processamento
    raw.c_oflag &= ~(OPOST);

    //control modes - set 8 bit chars
    raw.c_cflag |= (CS8);

    //local modes, choing off, canonical off, no extended functions, no signal chars (, etc)
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    //control chars - set return condition: min number of bytes and a timer
    raw.c_cc[VMIN] = 0; // return each byte, or zero for a timeout
    raw.c_cc[VTIME] = 1; //100ms timeout

    // coloca o terminal em modo raw apos realiar flush
    if(tcsetattr(fd, TCSAFLUSH, &raw) < 0) {
        errno = ENOTTY;
        return -1;
    }
    Editor.rawmode = 1;
    return 0;
}

// Verifica se uma string so' contem numeros
int numbers_only(const char *s)
{
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }

    return 1;
}

// Le uma tecla do input no modo eaw e realiza o tratamento
int editor_read_key(int fd)
{
    int nread;
    char c, seq[3];
    while((nread = read(fd, &c, 1)) == 0);
    if(nread == -1) exit(1);

    while(1) {
        switch(c) {
            case ESC: // sequencia de escape
                // se for um escape, entao timeout
                if(read(fd, seq, 1) == 0) return ESC;
                if(read(fd, seq+1, 1) == 0) return ESC;

                // ESC [ sequencias
                if(seq[0] == '[') {
                    if(seq[1] >= '0' && seq[1] <= '9') {
                        // escape estenddo, entao ler um byte adicional 
                        if(read(fd, seq+2, 1) == 0) return ESC;
                        if(seq[2] == '~') {
                            switch(seq[1]) {
                                case '3': return DEL_KEY;
                                case '5': return PAGE_UP;
                                case '6': return PAGE_DOWN;
                            }
                        }
                    } else {
                        switch(seq[1]) {
                            case 'A': return ARROW_UP;
                            case 'B': return ARROW_DOWN;
                            case 'C': return ARROW_RIGHT;
                            case 'D': return ARROW_LEFT;
                            case 'H': return HOME_KEY;
                            case 'F': return END_KEY;
                        }
                    }
                }

                // ESC 0 sequencias
                else if(seq[0] == 'O') {
                    switch (seq[1]) {
                        case 'H': return HOME_KEY;
                        case 'F': return END_KEY;
                    }
                }
                break;

            default:
                return c;
        }
    }
}

// Usa o ESC [6n para procurar pela posicao horizontal do cursor e retorna-la.
// Em caso de erro, -1 e' retornado. Em caso de sucesso, a posicao do cursor
// e' armazenada em *rows, *columns e 0 e' retornado.
int get_cursor_pos(int ifd, int ofd, int *rows, int *columns)
{
    char buf[32];
    unsigned int i = 0;

    // informa a localizacao do cursor
    if (write(ofd, "\x1b[6n", 4) != 4) return -1;

    // le a resposta: ESC [ linhas; R colunas
    while(i < sizeof(buf)-1) {
        if( read(ifd, buf+i, 1) != 1) break;
        if(buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    // parse
    if (buf[0] != ESC || buf[1] != '[') return -1;
    if (sscanf(buf+2,"%d;%d",rows,columns) != 2) return -1;
    
    return 0;
}


// Pega o numero de linhas na janela
int get_window_size(int ifd, int ofd, int *rows, int *columns)
{
    struct winsize ws;

    if(ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        // ioctl() falhou, entao buscar no terminal 
        int original_row, original_column, retval;

        // pega a posicao inicial e armazena para posteriormente
        retval = get_cursor_pos(ifd, ofd, &original_row, &original_column);
        if(retval == -1) {
            return -1;
        }

        // vai para a margem inferior direita e pega a posicao
        if(write(ofd, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        retval = get_cursor_pos(ifd, ofd, rows, columns);
        if(retval == -1) {
            return -1;
        }

        // restaura a posicao do cursor
        char seq[32];
        snprintf(seq, 32, "\x1b[%d;%dH", original_row, original_column);
        if(write(ofd, seq, strlen(seq)) == -1) {
            // nao conseguiu recuperar a posicao do cursor
        }
        return 0;
    } else {
        *columns = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

// SYNTAX HIGHLIGHTING

int is_separator(int c)
{
    return c == '\0' || isspace(c) || strchr(",.()+_/*=%[];", c) != NULL;
}

// retorna TRUE se o ultimo caracter da linha especificada e' parte de um comentario multi-linha que engloba a proxima linha
int editing_row_has_open_comment(editing_row *row)
{
    if(row->hl && row->rendered_size && row->hl[row->rendered_size-1] == HL_MLCOMMENT && 
      (row->rendered_size < 2 || (row->rendered_chars[row->rendered_size-2] != '*' || row->rendered_chars[row->rendered_size-1] != '/'))) return 1;

    return 0;
}

// coloca cada byte de row->hl (todos os caracteres na linha) para a sintaxe correta definida por HL_*
void editor_update_syntax(editing_row *row)
{
    row->hl = realloc(row->hl, row->rendered_size);
    memset(row->hl, HL_NORMAL, row->rendered_size);

    if(Editor.syntax == NULL) return; // nenhuma sintaxe nessa linha, tudo e' HL_NORMAL

    int i, prev_sep, in_string, in_comment;
    char *p;
    char **keywords = Editor.syntax->keywords;
    char *scs = Editor.syntax->singleline_comment_start;
    char *mcs = Editor.syntax->multiline_comment_start;
    char *mce = Editor.syntax->multiline_comment_end;

    // aponta para o primeiro caracter diferente de espaco em branco
    p = row->rendered_chars;
    i = 0; // offset do caractere atual na linha
    while (*p && isspace(*p)) {
        p++;
        i++;
    }
    prev_sep = 1; // diz as parser de 'i' aponta para o comeco de uma palavra 
    in_string = 0; // o caractere em uma string e' "" ou '' ?
    in_comment = 0; // o caractere esta em um comentario multi-linha aberto?

    // se a linha anterior possui um comentario aberta entao esta linha comeca com um estado de comentario aberto 
    if(row->index > 0 && editing_row_has_open_comment(row->prev)) in_comment = 1;

    while (*p) {
        // lida com comentario de linha unica
        if(prev_sep && *p == scs[0] && *(p+1) == scs[1]) {
            // a partir daqui ate' o final da linha e' um comentario 
            memset(row->hl+i, HL_COMMENT, row->rendered_size-i);
            return;
        }

        // linha com comentarios multi-linha
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

        // lida com "" e ''
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

        // lida com caracteres nao imprimiveis
        if(!isprint(*p)) {
            row->hl[i] = HL_NONPRINT;
            p++; i++;
            prev_sep = 0;
            continue;
        }

        // lida com numeros
        if((isdigit(*p) && (prev_sep || row->hl[i-1] == HL_NUMBER)) || (*p == '.' && i > 0 && row->hl[i-1] == HL_NUMBER)) {
            row->hl[i] = HL_NUMBER;
            p++; i++;
            prev_sep = 0;
            continue;
        }

        // lida com palavras-chave e chamadas a libs 
        if(prev_sep) {
            int j;
            for(j = 0; keywords[j]; j++) {
                int klen = strlen(keywords[j]);
                int pp = keywords[j][klen-1] == '|'; // palavras-chave do preprocessador
                int cond = keywords[j][klen-1] == '~'; // condicao
                int retu = keywords[j][klen-1] == '#';
                int adapter = keywords[j][klen-1] == '^'; // palavras-chave do adaptador
                int loopy = keywords[j][klen-1] == '@';
                if(pp || cond || retu || adapter || loopy) klen--;

                if(!memcmp(p, keywords[j], klen) && is_separator(*(p+klen))) {
                    // palavra-chave
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
                continue; // ocorreu um match de palavra-chave
            }
        }

        // caracteres nao especiais
        prev_sep = is_separator(*p);
        p++; i++;
    }

    // propaga a mudanca de sintaxe para a proxima linha se o estado do comentario aberto mudou 
    int oc = editing_row_has_open_comment(row);
    if(row->hl_open_comment != oc && row->index+1 < Editor.num_of_rows) editor_update_syntax(row->next);
    row->hl_open_comment = oc;
}

// Mapeia o highlight de sintaxe para cores do terminal 
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
}


// Seleciona o highlight de sintaxe de acordo com o nome do arquivo 
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
}

// IMPLEMENTACAO DAS LINHAS DO EDITOR

// Atualiza a versao renderizada e o highlight de sintaxe da linha 
void editor_update_row(editing_row *row)
{
    int tabs = 0, nonprint = 0, j, index;

    // cria uma versao da linha que possa ser printada diretamente na tela 
    free(row->rendered_chars);
    for(j = 0; j < row->size; j++)
        if( row->chars[j] == TAB) tabs++;

    row->rendered_chars = malloc(row->size + tabs*8 + nonprint*9 + 1);
    index = 0;
    for(j = 0; j < row->size; j++) {
        if(row->chars[j] == TAB) {
            row->rendered_chars[index++] = ' ';
            while((index+1) % 8 != 0) row->rendered_chars[index++] = ' ';
        } else {
            row->rendered_chars[index++] = row->chars[j];
        }
    }
    row->rendered_size = index;
    row->rendered_chars[index] = '\0';

    // atualiza o highlight de sintaxe para a linha 
    editor_update_syntax(row);
}

// Procura por um numero de linha e retorna esta linha caso encontre
editing_row *find_row(int at)
{
	editing_row *ret;
	if (Editor.current != NULL)
		ret = Editor.current;
	else
		ret = Editor.row_head;
    while (ret != NULL && ret->index != at)
    {
        if (ret->index > at)
		ret = ret->prev;
        else if (ret->index < at)
		ret = ret->next;
    }
    if (ret != NULL)
	    Editor.current = ret;
    return ret;
}

// Insere linha
void editor_insert_row(int at, char *s, size_t length)
{
    if(at > Editor.num_of_rows) return;

    editing_row *new = (editing_row*)malloc(sizeof(editing_row));

	  for(int j = at+1; j <= Editor.num_of_rows; j++) Editor.row[j].index++;*/
	if (Editor.num_of_rows == 0)
	{
		Editor.row_head = new;
		Editor.row_tail = NULL;
		Editor.row_head->prev = Editor.row_head->next = NULL;
	}
	else if (at == Editor.num_of_rows)
	{
		if (Editor.row_tail == NULL)
		{
			Editor.row_tail = new;
			new->prev = Editor.row_head;
			new->next = NULL;
			Editor.row_head->next = new;
		}
		else
		{
			Editor.row_tail->next = new;
			new->prev = Editor.row_tail;
			Editor.row_tail = Editor.row_tail->next;
			new->next = NULL;
		}
	}
	else
	{
		if (at == 0)
		{
			Editor.row_head->prev = new;
			new->next = Editor.row_head;
			Editor.row_head = Editor.row_head->prev;
		}
		else
		{
			editing_row *row = find_row(at);
			new->next = row;
			new->prev = row->prev;
			if (new->prev)
				new->prev->next = new;
			row->prev = new;
		}
		for (editing_row *i = new->next; i != NULL; i = i->next)
		{
			++i->index;
		}
	}

        new->size = length;
        new->chars = malloc(length+1);
        memcpy(new->chars, s, length+1);
        new->hl = NULL;
        new->hl_open_comment = 0;
        new->rendered_chars = NULL;
        new->rendered_size = 0;
        new->index = at;
        editor_update_row(new);
        ++Editor.num_of_rows;
        if(Editor.num_of_rows)
		  line_number_length = 3 + (int)log10((double)Editor.num_of_rows);
        Editor.dirty++;
        Editor.current = new;
}

// Libera a heap de linha alocadas 
void editor_free_row(editing_row *row)
{
    free(row->rendered_chars);
    free(row->chars);
    free(row->hl);
}

// Remove a linha em uma posicao especificada 
void editor_delete_row(int at)
{
    editing_row *row;

    if(at >= Editor.num_of_rows) return;
    row = find_row(at);

    if (Editor.row_head == row)
    {
        Editor.row_head = row->next;
    }
    if (Editor.row_tail == row)
    {
        Editor.row_tail = row->prev;
    }
    if (row->prev)
        row->prev->next = row->next;
    if (row->next)
        row->next->prev = row->prev;

    for (editing_row *i = row->next; i != NULL; i = i->next)
    {
        --i->index;
    }

    editor_free_row(row);

    --Editor.num_of_rows;
	if(Editor.num_of_rows) {
		line_number_length = 3 + (int)log10((double)Editor.num_of_rows);
	}
	else {
		line_number_length = 3;
	}
    ++Editor.dirty;
}

// Transforma as linhas do editor em uma unica string alocada em heap 
char *editor_rows_to_string(int *buflen)
{
    char *buf = NULL, *p;
    int totlen = 0;
    editing_row *j;

    // Contagem de bytes
    for(j = Editor.row_head; j != NULL; j = j->next)
        totlen += j->size+1; // +1 e' para cada \n ao final da linha
    *buflen = totlen;
    totlen++; // libera espaco para o NULL

    p = buf = malloc(totlen);
    for(j = Editor.row_head; j != NULL; j = j->next) {
        memcpy(p, j->chars, j->size);
        p += j->size;
        *p = '\n';
        p++;
    }
    *p = '\0';
    return buf;
}

// Insere um caractere em uma posicao especificada na linha
void editor_row_insert_char(editing_row *row, int at, int c)
{
    if(at > row->size) {
        // faz o padding na string com espacos se a posicao se insercao esta fora do 
        // comprimento atual da linha por mais que um caractere 
        int padlen = at-row->size;

        row->chars = realloc(row->chars, row->size+padlen+2); // +2 para NULL e \n
        memset(row->chars+row->size, ' ', padlen);
        row->chars[row->size+padlen+1] = '\0';
        row->size += padlen+1;
    } else {
        // no meio da string, entao libera espaco para o caraceter 
        row->chars = realloc(row->chars, row->size+2);
        memmove(row->chars+at+1, row->chars+at, row->size-at+1);
        row->size++;
    }
    row->chars[at] = c;
    editor_update_row(row);
    Editor.dirty++;
}

// Append a string no fim da linha 
void editor_row_append_string(editing_row *row, char *s, size_t len)
{
    row->chars = realloc(row->chars, row->size+len+1);
    memcpy(row->chars+row->size, s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    editor_update_row(row);
    Editor.dirty++;
}

// Deleta o caracter no offset 'at' na linha especificada 
void editor_row_delete_char(editing_row *row, int at)
{
    if(row->size <= at) return;
    memmove(row->chars+at, row->chars+at+1, row->size-at);
    editor_update_row(row);
    row->size--;
    Editor.dirty++;
}

// Insere um caractere na posicao corrente do cursor 
void editor_insert_char(int c)
{
    int filerow = Editor.row_offset+Editor.cursor_y;
    int filecol = Editor.column_offset+Editor.cursor_x;
    editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);

    // se a linha na qual o cursor esta localizado nao existe, adicionar quantas linhas vazias forem necessarias 
    if(!row) {
        while(Editor.num_of_rows <= filerow)
            editor_insert_row(Editor.num_of_rows, "", 0);
        row = find_row(filerow);
    }

    editor_row_insert_char(row, filecol, c);
    if(Editor.cursor_x == Editor.screen_columns-1)
        Editor.column_offset++;
    else
        Editor.cursor_x++;
    Editor.dirty++;
}

// Pega o prefixo de identacao da linha
char *get_indent_prefix(char *s, int max_length)
{
    int ptr = 0;
    while (s[ptr] == ' ' || s[ptr] == '\t') ptr++;
    if (ptr > max_length) ptr = max_length;
    char *res = (char*)malloc((ptr + 1) * sizeof(char));
    memcpy(res, s, ptr * sizeof(char));
    res[ptr] = '\0';
    return res;
}

// Define o prefixo de identacao da linha
char *set_indent_prefix(char *text, char *prefix)
{
    int result_length = strlen(text) + strlen(prefix);
    int ptr = 0;
    while (text[ptr] == ' ' || text[ptr] == '\t') ptr++;
    result_length -= ptr;
    char *res = (char*)malloc((result_length + 1) * sizeof(char));
    res[0] = '\0';
    res = strcat(res, prefix);
    res = strcat(res, text + ptr);
    return res;
}

// Copia a linha atual para um buffer separado
int editor_copy_row() 
{
    int filerow = Editor.row_offset+Editor.cursor_y;

    editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);

    if (row) {
        Editor.yank_buffer_len = row->size+1;
        if (!Editor.yank_buffer) {
            if ((Editor.yank_buffer = (char*) malloc(Editor.yank_buffer_len)) != NULL ) {
                memcpy(Editor.yank_buffer, row->chars, Editor.yank_buffer_len);
            } else {
                perror("Error allocating memory.");
                exit(1);
            }
        } else {
            if ((Editor.yank_buffer = (char*) realloc(Editor.yank_buffer, Editor.yank_buffer_len)) != NULL ) {
                memcpy(Editor.yank_buffer, row->chars, Editor.yank_buffer_len);
            } else {
                perror("Error allocating memory.");
                exit(1);
            }
        }
	   return 1;
    }
    return 0;
}

// Empurra a linha para baixo
void editor_yank_row() 
{
    if(editor_copy_row()) {
    	int filerow = Editor.row_offset+Editor.cursor_y;
    	editor_delete_row(filerow);
    }
}

// Cola uma linha na posicao do cursor
void editor_paste_row() 
{
    int filerow = Editor.row_offset+Editor.cursor_y;
    editor_insert_row(filerow, Editor.yank_buffer, Editor.yank_buffer_len-1); // -1 para comensar terminador NULL
}

// Inserir um caracter de nova linha e' mais complexo, deve-se lidar com
// a insercao no meio de uma linha, dividindo-a conforme necessario
void editor_insert_newline(void)
{
    int filerow = Editor.row_offset+Editor.cursor_y;
    int filecol = Editor.column_offset+Editor.cursor_x;
    editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);
    char *indent_prefix = NULL;

    if (!row) {
        if(filerow == Editor.num_of_rows) {
            editor_insert_row(filerow, "", 0);
            goto fixcursor;
        }
        return;
    }

    if (Editor.indent)
    {
        indent_prefix = get_indent_prefix(row->chars, filecol + 1);
    }
    else
    {
        indent_prefix = get_indent_prefix(row->chars, 0);
    }
    // se o cursor esta alem do tamanho da linha, entao esta no ultimo caractere
    if(filecol >= row->size) filecol = row->size;
    if(filecol == 0) {
        editor_insert_row(filerow, indent_prefix, strlen(indent_prefix));
    } else {
        // meio da linha, separa-la em duas linhas
        char *new_row = set_indent_prefix(row->chars+filecol, indent_prefix);
        editor_insert_row(filerow+1, new_row, strlen(new_row));
        free(new_row);
        row = find_row(filerow);
        row->chars[filecol] = '\0';
        row->size = filecol;
        editor_update_row(row);
    }

fixcursor:
    if(Editor.cursor_y == Editor.screen_rows-1)
        Editor.row_offset++;
    else
        Editor.cursor_y++;

    if(indent_prefix) {
        Editor.cursor_x = strlen(indent_prefix) % Editor.screen_columns;
        Editor.column_offset = strlen(indent_prefix) / Editor.screen_columns;
        free(indent_prefix);
    }
}

// Deleta o caractere na posicao autal 
void editor_delete_char()
{
    int filerow = Editor.row_offset+Editor.cursor_y;
    int filecol = Editor.column_offset+Editor.cursor_x;
    editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);

    if(!row || (filecol == 0 && filerow == 0)) return;

    if(filecol == 0) {
        // Se a coluna = 0, entao mover a linha corrente para a direita da anterior
        filecol = row->prev->size;
        editor_row_append_string(row->prev, row->chars, row->size);
        editor_delete_row(filerow);
        row = NULL;
        if(Editor.cursor_y == 0)
            Editor.row_offset--;
        else
            Editor.cursor_y--;
        Editor.cursor_x = filecol;
        if(Editor.cursor_x >= Editor.screen_columns) {
            int shift = (Editor.screen_columns-Editor.cursor_x)+1;
            Editor.cursor_x -= shift;
            Editor.column_offset += shift;
        }
    } else {
        editor_row_delete_char(row, filecol-1);
        if(Editor.cursor_x == 0 && Editor.column_offset)
            Editor.column_offset--;
        else
            Editor.cursor_x--;
    }

    if(row) editor_update_row(row);
    Editor.dirty++;
}

// parser dos argumentos
void parse_argument(char *arg)
{
    int ptr = 1;
    while (arg[ptr] != '\0')
    {
        switch (arg[ptr])
        {
            case 'l':
                Editor.line_numbers = 1;
                break;
            case 'i':
                Editor.indent = 1;
                break;
            default:
                fprintf(stderr, "bric: invalid option -- '%c'\n", arg[ptr]);
                exit(1);
        }
        ptr++;
    }
}

// Carrega o arquivo especificado na memoria do editor 
int editor_open(char *filename)
{
    FILE *fp;
    Editor.dirty = 0;
    fp = fopen(filename, "r");
    if(!fp) {
        if(errno != ENOENT) {
                perror("Opening file");
                exit(1);
        }
    	// insere uma linha se o arquvo e' novo (vazio)
    	editor_insert_row(Editor.num_of_rows, "", 0);
    	Editor.dirty = 0;
    	Editor.newfile = 1;
        return 1;
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while((linelen = getline(&line, &linecap, fp)) != -1) {
        if(linelen && (line[linelen-1] == '\n' || line[linelen-1] == '\r'))
            line[--linelen] = '\0';
        editor_insert_row(Editor.num_of_rows, line, linelen);
    }

    free(line);
    fclose(fp);
    Editor.dirty = 0;
    return 0;
}

// Salva o arquivo atual em disco
int editor_save(void)
{
    int len;
    char *buf = editor_rows_to_string(&len);
    int fd = open(Editor.filename, O_RDWR|O_CREAT, 0644);
    if(fd == -1) goto writeerr;

    // Usa truncate junto a uma unica chamada a write para tornar o salvamento
    // mais seguro, nos limites do que pode ser feito em um editor pequeno
    if(ftruncate(fd, len) == -1) goto writeerr;
    if(write(fd, buf, len) != len) goto writeerr;

    close(fd);
    free(buf);
    Editor.dirty = 0;
    Editor.newfile = 0;
    editor_set_status_message("%d bytes written on disk", len);
    return 0;

writeerr:
    free(buf);
    if(fd != -1) close(fd);
    editor_set_status_message("Cannot save! I/O error: %s", strerror(errno));
    return 1;
}

// ATUALIZACAO DO TERMINAL

// Armazena o caractere ab passado no terminal
void ab_append(struct append_buf *ab, char *s, int length)
{
    char *new = realloc(ab->b, ab->length+length);

    if(new == NULL) return;
    memcpy(new+ab->length, s, length);
    ab->b = new;
    ab->length += length;
}

// Libera memoria do ab alocado
void ab_free(struct append_buf *ab)
{
    free(ab->b);
}

// Realiza a copia do que esta no editor para a area de transferencia
void copy_to_clipboard(void)
{
    free(Editor.clipboard);
    int lx = Editor.cursor_x + Editor.column_offset;
    int ly = Editor.cursor_y + Editor.row_offset;
    int rx = Editor.selected_base_x;
    int ry = Editor.selected_base_y;
    if (ly > ry || (ly == ry && lx > rx))
    {
        int t = lx;
        lx = rx;
        rx = t;
        t = ly;
        ly = ry;
        ry = t;
    }
    int clipboard_length = 1;
    editing_row *row = find_row(ly);
    for (int i = ly; i <= ry; i++)
    {
        int left_border = 0, right_border = row->size - 1;
        if (i == ly) left_border = lx;
        if (i == ry && rx < right_border) right_border = rx;
        clipboard_length += right_border - left_border + 1;
        if (i != ry) clipboard_length++;
        row = row->next;
    }
    Editor.clipboard = (char*)malloc(clipboard_length * sizeof(char));
    int ptr = 0;
    row = find_row(ly);
    for (int i = ly; i <= ry; i++)
    {
        int left_border = 0, right_border = row->size - 1;
        if (i == ly) left_border = lx;
        if (i == ry && rx < right_border) right_border = rx;
        for (int j = left_border; j <= right_border; j++)
        {
            Editor.clipboard[ptr] = row->chars[j];
            ptr++;
        }
        if (i != ry)
        {
            Editor.clipboard[ptr] = '\n';
            ptr++;
        }
    }
    Editor.clipboard[ptr] = '\0';
}

// Realiza a cola do que esta na area de transferencia para o editor
void paste_from_clipboard(void)
{
    if (Editor.clipboard == NULL) return;
    int length = strlen(Editor.clipboard);
    for (int i = 0; i < length; i++)
    {
        if (Editor.clipboard[i] == '\n')
        {
            editor_insert_newline();
        }
        else
        {
            editor_insert_char(Editor.clipboard[i]);
        }
    }
}

// Verifica se um caracter esta selecionado
int is_char_selected(int x, int y)
{
    int lx = Editor.cursor_x + Editor.column_offset;
    int ly = Editor.cursor_y + Editor.row_offset;
    int rx = Editor.selected_base_x;
    int ry = Editor.selected_base_y;
    if (ly > ry || (ly == ry && lx > rx))
    {
        int t = lx;
        lx = rx;
        rx = t;
        t = ly;
        ly = ry;
        ry = t;
    }
    if (ly == ry)
    {
        return y == ly && x >= lx && x <= rx;
    }
    else if (y == ly)
    {
        return x >= lx;
    }
    else if (y == ry)
    {
        return x <= rx;
    }
    else
    {
        return y >= ly && y <= ry;
    }
}

// Escreve em toda a tela usando os caracteres de escape VT100
// comecando pelo estado logico do edito no estado global 'Editor'
void editor_refresh_screen(void)
{
    int y;
    editing_row *row;
    char buf[32];
    struct append_buf ab = ABUF_INIT;

    ab_append(&ab, "\x1b[?25l", 6); // esconde o cursor
    ab_append(&ab, "\x1b[H", 3); // vai home
    for(y = 0; y < Editor.screen_rows; y++) {
        int filerow = Editor.row_offset+y;
        ab_append(&ab, "\x1b[49m", 5);
        ab_append(&ab, "\x1b[39m", 5);

        if (Editor.line_numbers && filerow < Editor.num_of_rows)
        {
            sprintf(buf, line_number_format[line_number_length - 3], filerow + 1);
            ab_append(&ab, buf, strlen(buf));
	        ab_append(&ab, ": ", 2);
        }
        if(filerow >= Editor.num_of_rows) {
            if(Editor.num_of_rows == 1 && y == Editor.screen_rows/3 && Editor.mode != INSERT_MODE && !Editor.dirty && Editor.newfile) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome), "Bric editor -- version %s\x1b[0K\r\n", BRIC_VERSION);
                int padding = (Editor.screen_columns-welcomelen)/2;
                if(padding) {
                    ab_append(&ab, "~", 1);
                    padding--;
                }

                while(padding--) ab_append(&ab, " ", 1);
                ab_append(&ab, welcome, welcomelen);
            } else {
                ab_append(&ab, "~\x1b[0K\r\n", 7);
            }

            continue;
        }
        row = find_row(filerow);
        int len = row->rendered_size - Editor.column_offset;
        int current_colour = -1, background_colour = -1;
        if(len > 0) {
            if(len > Editor.screen_columns) len = Editor.screen_columns;
            char *c = row->rendered_chars+Editor.column_offset;
            unsigned char *hl = row->hl+Editor.column_offset;
            int j;
            for (j = 0; j < len; j++) {
                if (Editor.mode == SELECTION_MODE && is_char_selected(j + Editor.column_offset, filerow)) {
                    if (background_colour == -1)
                    {
                        ab_append(&ab, "\x1b[47m", 5);
                        background_colour = 47;
                    }
                    if (current_colour != 30)
                    {
                        ab_append(&ab, "\x1b[30m", 5);
                        current_colour = 30;
                    }
                    ab_append(&ab, c+j, 1);

                } else if(hl[j] == HL_NONPRINT) {
                    if (background_colour != -1)
                    {
                        ab_append(&ab, "\x1b[49m", 5);
                        background_colour = -1;
                    }
                    char sym;
                    ab_append(&ab, "\x1b[7m", 4);
                    if(c[j] <= 26)
                            sym = '@'+c[j];
                    else
                            sym = '?';
                    ab_append(&ab, &sym, 1);
                    ab_append(&ab, "\x1b[0m", 4);
                } else if (hl[j] == HL_NORMAL) {
                    if (background_colour != -1)
                    {
                        ab_append(&ab, "\x1b[49m", 5);
                        background_colour = -1;
                    }
                    if(current_colour != -1) {
                            ab_append(&ab, "\x1b[39m", 5);
                            current_colour = -1;
                    }
                    ab_append(&ab, c+j, 1);
                } else {
                    int colour = editor_syntax_to_colour(hl[j]);
                    if (background_colour != -1)
                    {
                        ab_append(&ab, "\x1b[49m", 5);
                        background_colour = -1;
                    }
                    if(colour != current_colour) {
                        char buf[16];
                        int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", colour);
                        current_colour = colour;
                        ab_append(&ab, buf, clen);
                    }
                    ab_append(&ab, c+j, 1);
                }
            }
        }
        ab_append(&ab, "\x1b[39m", 5);
        ab_append(&ab, "\x1b[0K", 4);
        ab_append(&ab, "\r\n", 2);
    }

    // cria um status de duas linhas
    // primeira linha
    ab_append(&ab, "\x1b[0K", 4);
    ab_append(&ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.50s =- %d lines %s", Editor.filename, Editor.num_of_rows, Editor.dirty ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", Editor.row_offset+Editor.cursor_y+1, Editor.num_of_rows);
    if(len > Editor.screen_columns) len = Editor.screen_columns;
    ab_append(&ab, status, len);
    while(len < Editor.screen_columns) {
        if(Editor.screen_columns -len == rlen) {
            ab_append(&ab, rstatus, rlen);
            break;
        } else {
            ab_append(&ab, " ", 1);
            len++;
        }
    }
    ab_append(&ab, "\x1b[0m\r\n", 6);

    // segunda linha depende de Editor.status_message e tempo de atualizacao
    ab_append(&ab, "\x1b[0K", 4);
    int message_length = strlen(Editor.status_message);
    if(message_length && time(NULL)-Editor.status_message_time < 5)
        ab_append(&ab, Editor.status_message, message_length <= Editor.screen_columns ? message_length : Editor.screen_columns);

    // coloca o cursor na sua posicao atual
    int j;
    int cursor_x = 1;
    if (Editor.line_numbers) cursor_x += line_number_length;
    int filerow = Editor.row_offset+Editor.cursor_y;
    editing_row *r = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);
    if(r) {
        int was_tab = 0;
        for(j = Editor.column_offset; j < (Editor.cursor_x+Editor.column_offset); j++) {
                if(j < r->size && r->chars[j] == TAB)
                {
                    cursor_x += 7-((cursor_x)%8);
                    was_tab = 1;
                }
                cursor_x++;
        }
        if (was_tab) cursor_x--;
    }
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", Editor.cursor_y + 1, cursor_x);
    ab_append(&ab, buf, strlen(buf));
    ab_append(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.length);
    ab_free(&ab);
}

// coloca uma mensagem de status do editor para a segunda linha de status no fim da linha
void editor_set_status_message(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(Editor.status_message, sizeof(Editor.status_message), fmt, ap);
    va_end(ap);
    Editor.status_message_time = time(NULL);
}

// MODO DE PROCURA

void editor_find(int fd)
{
    char query[BRIC_QUERY_LENGTH+1] = {0};
    int qlen = 0;
    int last_match = -1; // ultima linha na qual o match foi encontrado, -1 para nenhuma
    int find_next = 0; // se 1, procurar proximo; se -1, procurar anterior 
    int saved_hl_line = -1;
    char *saved_hl = NULL;

    #define FIND_RESTORE_HL do { \
            if(saved_hl) { \
                memcpy(find_row(saved_hl_line)->hl, saved_hl, find_row(saved_hl_line)->rendered_size); \
                saved_hl = NULL; \
            } \
        } while (0)

    // salva a posicao do cursor para restaurar depois
    int saved_cursor_x = Editor.cursor_x, saved_cursor_y = Editor.cursor_y;
    int saved_column_offset = Editor.column_offset, saved_row_offset = Editor.row_offset;

    while(1) {
        editor_set_status_message("Search: %s (Use ESC/Arrows/Enter)", query);
        editor_refresh_screen();

        int c = editor_read_key(fd);
        if(c == DEL_KEY || c == CTRL_H || c == BACKSPACE) {
            if(qlen != 0) query[--qlen] = '\0';
            last_match = -1;
        } else if(c == ESC || c == ENTER) {
            if(c == ESC) {
                Editor.cursor_x = saved_cursor_x; Editor.cursor_y = saved_cursor_y;
                Editor.column_offset = saved_column_offset; Editor.row_offset = saved_row_offset;
            }
            FIND_RESTORE_HL;
            editor_set_status_message("");
            return;
        } else if (c == ARROW_RIGHT || c == ARROW_DOWN) {
            find_next = 1;
        } else if(c == ARROW_LEFT || c == ARROW_UP) {
            find_next = -1;
        } else if(isprint(c)) {
            if(qlen < BRIC_QUERY_LENGTH) {
                    query[qlen++] = c;
                    query[qlen] = '\0';
                    last_match = -1;
            }
        }

        // procura ocorrencia
        if(last_match == -1) find_next = 1;
        if(find_next) {
            char *match = NULL;
            int match_offset = 0;
            int i, current = last_match;

            editing_row *tmp = Editor.row_head;
            for (i = 0; i < Editor.num_of_rows; i++) {
                current += find_next;
                tmp = find_row(current);
                if(current == -1)
                {
                    current = Editor.num_of_rows-1;
                    tmp = Editor.row_tail;
                }
                else if (current == Editor.num_of_rows)
                {
                    current = 0;
                    tmp = Editor.row_head;
                }
                match = strstr(tmp->rendered_chars, query);
                if(match) {
                    match_offset = match-tmp->rendered_chars;
                    break;
                }
            }
            find_next = 0;


            // Highlight
            FIND_RESTORE_HL;

            if(match) {
                editing_row *row = find_row(current);
                last_match = current;
                if(row->hl) {
                    saved_hl_line = current;
                    saved_hl = malloc(row->rendered_size);
                    memcpy(saved_hl, row->hl, row->rendered_size);
                    memset(row->hl+match_offset, HL_MATCH, qlen);
                    memset(row->hl+match_offset+qlen, HL_BACKGROUND_DEFAULT, qlen);
                }
                Editor.cursor_y = 0;
                Editor.cursor_x = match_offset;
                Editor.row_offset = current;
                Editor.column_offset = 0;
                //scroll horizontally as needed
                if(Editor.cursor_x > Editor.screen_columns) {
                    int diff = Editor.cursor_x - Editor.screen_columns;
                    Editor.cursor_x -= diff;
                    Editor.column_offset += diff;
                }
            }
        }
    }
}

// MODO SUBSTITUICAO

void editor_find_replace(int fd)
{
	char query[BRIC_QUERY_LENGTH + 1] = { 0 };
	char replace_word[BRIC_QUERY_LENGTH + 1] = { 0 };
	int replace_len = 0;
	int qlen = 0;
	int last_match = -1; // ultima linha na qual o match foi encontrado, -1 para nenhuma
	int find_next = 0; // se 1, procurar proximo; se -1, procurar anterior 
	int saved_hl_line = -1; 
	char *current_input = query;
	int *current_input_len = &qlen;
	char *saved_hl = NULL;

    #define FIND_RESTORE_HL do { \
        if(saved_hl) { \
                memcpy(find_row(saved_hl_line)->hl, saved_hl, find_row(saved_hl_line)->rendered_size); \
                saved_hl = NULL; \
        } \
    } while (0)

	// salva a posicao do cursor para restaurar depois
	int saved_cursor_x = Editor.cursor_x, saved_cursor_y = Editor.cursor_y;
	int saved_column_offset = Editor.column_offset, saved_row_offset = Editor.row_offset;

	while (1) {
		editor_set_status_message("Search: %s Replace: %s (Use ESC/Tab/Arrows/Enter)", query, replace_word);
		editor_refresh_screen();

		int c = editor_read_key(fd);
		if (c == CTRL_H || c == BACKSPACE) {
			if (*current_input_len != 0) current_input[--(*current_input_len)] = '\0';
			last_match = -1;
		}
		else if (c == ESC) {
			if (c == ESC) {
				Editor.cursor_x = saved_cursor_x; Editor.cursor_y = saved_cursor_y;
				Editor.column_offset = saved_column_offset; Editor.row_offset = saved_row_offset;
			}
			FIND_RESTORE_HL;
			editor_set_status_message("");
			return;
		}
		else if (c == ARROW_RIGHT || c == ARROW_DOWN) {
			find_next = 1;
		}
		else if (c == ARROW_LEFT || c == ARROW_UP) {
			find_next = -1;
		}
		else if (c == TAB) {
			if (current_input == query) {
				current_input = replace_word;
				current_input_len = &replace_len;
			} else if(current_input == replace_word) {
				current_input = query;
				current_input_len = &qlen;
			}
		}
		else if (c == ENTER) {
			Editor.cursor_x += qlen;
			for (int i = 0; i < qlen; i++)
			{
				editor_delete_char();
			}
			for (int j = 0; j < replace_len; j++)
			{
				editor_insert_char(replace_word[j]);
			}
                       editor_refresh_screen();
                       return;
		}
		else if (isprint(c)) {
			if (qlen < BRIC_QUERY_LENGTH) {
				current_input[(*current_input_len)++] = c;
				current_input[(*current_input_len)] = '\0';
				last_match = -1;
			}
		}

		// procura a ocorrencia
		if (last_match == -1) find_next = 1;
		if (find_next) {
			char *match = NULL;
			int match_offset = 0;
			int i, current = last_match;

            editing_row *tmp = Editor.row_head;
			for (i = 0; i < Editor.num_of_rows; i++) {
				current += find_next;
                tmp = find_row(current);
				if (current == -1){
			        current = Editor.num_of_rows - 1;
			        tmp = Editor.row_tail;
				 }
				else if (current == Editor.num_of_rows){
			        current = 0;
			        tmp = Editor.row_head;
				}

				match = strstr(tmp->rendered_chars, query);
				if (match) {
					match_offset = match - tmp->rendered_chars;
					break;
				}
			}
			find_next = 0;


			// Highlight
			FIND_RESTORE_HL;

			if (match) {
				editing_row *row = find_row(current);
				last_match = current;
				if (row->hl) {
					saved_hl_line = current;
					saved_hl = malloc(row->rendered_size);
					memcpy(saved_hl, row->hl, row->rendered_size);
					memset(row->hl + match_offset, HL_MATCH, qlen);
					memset(row->hl + match_offset + qlen, HL_BACKGROUND_DEFAULT, qlen);
				}
				Editor.cursor_y = 0;
				Editor.cursor_x = match_offset;
				Editor.row_offset = current;
				Editor.column_offset = 0;
				// scroll na horizontal caso seja necessario
				if (Editor.cursor_x > Editor.screen_columns) {
					int diff = Editor.cursor_x - Editor.screen_columns;
					Editor.cursor_x -= diff;
					Editor.column_offset += diff;
				}
			}
		}
	}
}


// TRATAMENTO DE EVENTOS DO EDITOR

//trar a mudanca de posicao do cursor devido ao pressionamento das setas do teclado
void editor_move_cursor(int key)
{
    int filerow = Editor.row_offset + Editor.cursor_y;
    int filecol = Editor.column_offset + Editor.cursor_x;
    int rowlen;
    editing_row *row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);

    switch(key) {
        case 'h':
        case ARROW_LEFT:
            if(Editor.cursor_x == 0) {
                if(Editor.column_offset)
                    Editor.column_offset--;
                else {
                    if(filerow > 0) {
                        Editor.cursor_y--;
                        Editor.cursor_x = row->prev->size;
                        if(Editor.cursor_x > Editor.screen_columns-1) {
                            Editor.column_offset = Editor.cursor_x-Editor.screen_columns+1;
                            Editor.cursor_x = Editor.screen_columns-1;
                        }
                    }
                }
            } else {
                Editor.cursor_x -= 1;
            }
            break;
        case 'l':
        case ARROW_RIGHT:
            if(row && filecol < row->size) {
                if(Editor.cursor_x == Editor.screen_columns-1) {
                    Editor.column_offset++;
                } else {
                    Editor.cursor_x += 1;
                }
            } else if ( row && filecol == row->size) {
        	/* Nao ir alem da ultima linha */
        	if(filerow == Editor.num_of_rows - 1) /* indexacao de 'filerow' comeca em 0 */ {
        		break;
        	}
                Editor.cursor_x = 0;
                Editor.column_offset = 0;
                if(Editor.cursor_y == Editor.screen_rows-1) {
                    Editor.row_offset++;
                } else {
                    Editor.cursor_y += 1;
                }
            }
            break;
        case 'k':
        case ARROW_UP:
            if(Editor.cursor_y == 0) {
                if(Editor.row_offset) Editor.row_offset--;
            } else {
                Editor.cursor_y -= 1;
            }
            break;
        case 'j':
        case ARROW_DOWN:
            if(filerow < Editor.num_of_rows - 1) //Não vai além da última linha. Indexação de 'filerow' começa em 0
                if(Editor.cursor_y == Editor.screen_rows-1) {
                    Editor.row_offset++;
                } else {
                    Editor.cursor_y += 1;
                }
            }
            break;
        case HOME_KEY:
            Editor.cursor_x=0;
            Editor.column_offset=0;
            break;
        case END_KEY:
            if(row->size>Editor.screen_columns-1)
            {
                Editor.cursor_x=Editor.screen_columns-1;
                Editor.column_offset=row->size-(Editor.screen_columns+1)+1;
            } else {
                Editor.cursor_x=row->size-1;
            }
            break;
        case PAGE_UP:
            if(Editor.cursor_y != 0) Editor.cursor_y = 0;
            {
                int times = Editor.screen_rows-2; // mantem as duas ultimas linhas da visualizacao anterior (como o VIM) 
                while(times--)
                    editor_move_cursor(ARROW_UP);
            }
            break;
        case PAGE_DOWN:
            if (Editor.cursor_y != Editor.screen_rows-1) Editor.cursor_y = Editor.screen_rows-1;
            {
                int times = Editor.screen_rows-2; // mantem as duas ultimas linhas da visualizacao anterior (como o VIM)
                while(times--)
                    editor_move_cursor(ARROW_DOWN);
            }
            break;
    }

    // arruma o cursor_x se a linha atual nao possui caracteres
    filerow = Editor.row_offset+Editor.cursor_y;
    filecol = Editor.column_offset+Editor.cursor_x;
    row = (filerow >= Editor.num_of_rows) ? NULL : find_row(filerow);
    rowlen = row ? row->size : 0;
    if(filecol > rowlen) {
        Editor.cursor_x -= filecol-rowlen;
        if (Editor.cursor_x < 0) {
            Editor.column_offset += Editor.cursor_x;
            Editor.cursor_x = 0;
        }
    }
}


// GOTO
// Realiza a movimentacao dentro do editor editor
void editor_goto(int linenumber)
{
	int line_number = linenumber;
	int current_line;

	while(1) {
		current_line = Editor.row_offset + Editor.cursor_y + 1;
		editor_refresh_screen();

		if(line_number <= Editor.num_of_rows && line_number > 0) {
			if (current_line > line_number) {
				int diff = current_line - line_number;

      				while(diff > 0) {
					editor_move_cursor(ARROW_UP);
					diff--;
				}
				editor_set_status_message("");
			} else if(line_number > current_line) {
				int diff = line_number - current_line;

				while(diff > 0) {
					editor_move_cursor(ARROW_DOWN);
					diff--;
				}
				editor_set_status_message("");
			}
			editor_refresh_screen();
			return;
		} else {
			editor_set_status_message("Out of bounds");
			editor_refresh_screen();
			return;
		}
	}
}

// Fecha o editor
void editor_harsh_quit()
{
    // Checagem se o arquivo esta travado, caso esteja, desbloqueia
    if(is_file_locked(CurrentFile)){
      unlock_file(CurrentFile);
    }

    exit(EXIT_SUCCESS);
}

/*
*	Função que controla a opção para sair do editor bric.
*	Passo 1: Mensagem de confirmação
*	Passo 2: Verifica a entrada do usuário
*		2.1) Se for 'DEL_KEY' ou 'BACKSPACE': apaga caracter escrito pelo usuário (se houver algum)
*		2.2) Se for 'ENTER': verifica o comando digitado. Se 'y', então sai do editor. Se 'n', não sai
*		2.3) Se for 'ESC': Não faz nada
*		2.4) Se for qualquer outro caracter: adiciona ao vetor 'query' (vetor de comando)
*/
void editor_check_quit(int fd)
{
        char query[BRIC_QUERY_LENGTH+1] = {0};
        int qlen = 0;

        while(1) {
                editor_set_status_message("There are unsaved changes, quit? (y or n) %s", query);
                editor_refresh_screen();

                int c = editor_read_key(fd);
                if(c == DEL_KEY || c == BACKSPACE) {					//PASSO 2.1
                        if(qlen != 0) query[--qlen] = '\0';
                }else if(c == ENTER || c == ESC) {					//PASSO 2.2 E 2.3
                        if(c == ENTER) {
                                if (strcmp(query, "y") == 0) editor_harsh_quit();
                                else if(strcmp(query, "n") == 0) return;
                                else {
                                        editor_check_quit(fd);
                                        return;
                                }
                        }
                } else if(isprint(c)) {							//PASSO 2.4
                        if(qlen < BRIC_QUERY_LENGTH) {
                                query[qlen++] = c;
                                query[qlen] = '\0';
                        }
                }
        }
}

// TAG MOVEMENT FUNCTIONS

// Função para diferenciar nome de identificadores com sintaxe de linguagem
int char_check(char c) {
	if(c == ' ' || c == '\n' || c == '\t' || c == '(' || c == ')' || c == '{' || c == '}')
		return 0;
	if(c == '&' || c == '|' || c == '+' || c == '-' || c == '*' || c == '\\' || c == '/' )
		return 0;
	if(c == '^' || c == '%' || c == '=' || c == '[' || c == ']' || c == ';' || c == '>' || c == '<')
		return 0;
	if(c == '?' || c == ':' || c == '\'' || c == '\"' || c == '.' || c == ',' || c == '!')
		return 0;
	return 1;
}

// Função para obter a palavra chave (identificador) na qual o cursor está posicionado
char* get_key(void) {
	int i = 0;
	int filerow = Editor.row_offset + Editor.cursor_y;
	int filecol = Editor.column_offset + Editor.cursor_x;
	if(!char_check(find_row(filerow)->chars[filecol]))
		return "";
	while(filecol != -1 && char_check(find_row(filerow)->chars[filecol]))
		filecol--;
	filecol++;
	char *key = (char*)malloc(128 * sizeof(char));
	while(char_check(find_row(filerow)->chars[filecol])) {
		key[i] = find_row(filerow)->chars[filecol];
		i++; 
		filecol++;
	}
	key[i] = '\0';
	return key;
}

// Função para alanisar as tags do arquivo, como por exemplo o comando de busca requisitada de uma linha em um arquivo
int tagsearch(char *tosearch) {
	int dest_index = 0, source_index = 2, i = 0;
	char *parsedsearch = (char*)malloc(strlen(tosearch) * sizeof(char));
	while(tosearch[source_index] != '$') {
		if(tosearch[source_index] == '\\') {
			source_index++;
			continue;
		}
		parsedsearch[dest_index] = tosearch[source_index];
		source_index++;
		dest_index++;
	}
	parsedsearch[dest_index] = '\0';
	for(i = 0; i < Editor.num_of_rows; i++) {
		if(strstr(find_row(i)->chars, parsedsearch) != NULL)
			return i + 1;
	}
	return 0;
}


// Principal função para manipular movimentos de tag
int handle_tag_movement(int where) {
	int linenumber = 0, i = 0;
	char *key = get_key();
	char tag_line[256], *tagname, *filename, *tosearch, *orig_filename;
	FILE *fp;
	int cursor_pos = Editor.cursor_y, cursor_offset = Editor.row_offset;
	tagdata tag_data;
	orig_filename = (char*)malloc(sizeof(char) * strlen(Editor.filename));
	strcpy(orig_filename, Editor.filename);
	if(where == MOVE_BACK) {
		if(isempty(&tag_stack)) {
			editor_set_status_message("at bottom of tag stack");
			return 1;
		}
		tag_data = pop(&tag_stack);
		linenumber = tag_data.linenumber;
		filename = (char*)malloc(sizeof(char) * strlen(tag_data.filename));
		strcpy(filename, tag_data.filename);
		if(strcmp(filename, Editor.filename)) {
                        if(Editor.dirty) {
                                editor_set_status_message("Unsaved changes. Can't proceed");
                                return 0;
                        }
                        editor_start(filename);
                }
		editor_goto(linenumber);
		
                //Posiciona a página para que o cursor permaneça na mesma linha 
		for(i = 0; i < cursor_pos && Editor.row_offset + Editor.cursor_y < Editor.num_of_rows - 1; i++)
			editor_move_cursor(ARROW_DOWN);
                for(; i > 0; i--)
			editor_move_cursor(ARROW_UP);
                for(i = 0; i < cursor_pos && Editor.row_offset + Editor.cursor_y > 0; i++)
			editor_move_cursor(ARROW_UP);
                for(; i > 0; i--)
			editor_move_cursor(ARROW_DOWN);
                return 1;		
	}
	if(key[0] == '\0') {
		editor_set_status_message("No identifier under cursor");
		return 0;
	}
	fp = fopen("tags", "r");
	if(!fp) {
		editor_set_status_message("tag not found: %s", key);
		free(key);
		return 0;
	}
	while(fgets(tag_line, 256, fp) != NULL) {
		tagname = strtok(tag_line, "\t");
		if(strcmp(tagname, key))
			continue;
		filename = strtok(NULL, "\t");
		tosearch = strtok(NULL, "\t");
		if(strcmp(filename, Editor.filename)) {
			if(Editor.dirty) {
				editor_set_status_message("Unsaved changes. Can't proceed");
				return 0;				
			}
			editor_start(filename);
		}
		if(tosearch[0] == '/') {
			linenumber = tagsearch(tosearch);
		}
		else {
			sscanf(tosearch, "%d", &linenumber);
		}
		tag_data.linenumber = cursor_offset + cursor_pos + 1;
		strcpy(tag_data.filename, orig_filename);
		push(&tag_stack, tag_data);
		editor_goto(linenumber);

                //Posiciona a pagina para que o cursor permaneça na mesma linha
		for(i = 0; i < cursor_pos && Editor.row_offset + Editor.cursor_y < Editor.num_of_rows - 1; i++)
			editor_move_cursor(ARROW_DOWN);
                for(; i > 0; i--)
			editor_move_cursor(ARROW_UP);
                for(i = 0; i < cursor_pos && Editor.row_offset + Editor.cursor_y > 0; i++)
			editor_move_cursor(ARROW_UP);
                for(; i > 0; i--)
			editor_move_cursor(ARROW_DOWN);
		return 1;
	}
	editor_set_status_message("tag not found: %s", key);
	free(key);
	return 0;
}

/*
*  Função que executa os comandos do editor
* Os comandos podem ser:
*	Passo 1: Se o comando for valor numérico -> muda o cursor para a linha desejada
*	Passo 2: Se o comando for 'q!' -> sair sem salvar
*	Passo 3: Se o comando for 'w' -> salvar
*	Passo 4: Se o comando for 'wq' -> salvar e sair
*	Passo 5: Se o comando for 'q' -> oferece opção de salvar para sair ou não sair 
*	Passo 6: Se o comando for 'f' -> 
*	Passo 7: Se o comando for 'fr' -> 
*	Passo 8: Se o comando for 'sm' -> modo para selecionar bloco de texto
*	Passo 9: Se o comando for 'sp' -> 
*	Passo 10: Se o comando for 'up' ->
*/
void editor_parse_command(int fd, char *query)
{
        if (numbers_only(query)) {			//PASSO 1
                editor_goto(atoi(query));
                return;
        } else if (strcmp(query, "q!") == 0) {		//PASSO 2
                editor_harsh_quit();
                return;
        } else if(strcmp(query, "w") == 0) {		//PASSO 3
                editor_save();
                return;
        } else if (strcmp(query, "wq") == 0) {		//PASSO 4
                editor_save();
                editor_harsh_quit();
                return;
        } else if (strcmp(query, "q") == 0) {		//PASSO 5
                if (Editor.dirty) {
                        editor_check_quit(fd);
                } else {
                        editor_harsh_quit();
                }
                return;
        } else if (strcmp(query, "f") == 0) {		//PASSO 6
            editor_find(fd);
            return;
        } else if (strcmp(query, "fr") == 0) {		//PASSO 7
            editor_find_replace(fd);
            return;
        } else if (strcmp(query, "sm") == 0) {		//PASSO 8
            Editor.mode = SELECTION_MODE;
            Editor.selected_base_x = Editor.cursor_x + Editor.column_offset;
            Editor.selected_base_y = Editor.cursor_y + Editor.row_offset;
            editor_set_status_message(selection_mode_message);
            return;
        } else if (strcmp(query, "sp") == 0) {		//PASSO 9
            Editor.indent = 0;
            return;
        } else if (strcmp(query, "up") == 0) {		//PASSO 10
            Editor.indent = 1;
            return;
        }
}

// Função que habilita a escrita de comandos (salvar, sair etc)
void enter_command(int fd)
{
        char query[BRIC_QUERY_LENGTH+1] = {0};
	int qlen = 0;

	while(1) {
		editor_set_status_message(":%s", query);
		editor_refresh_screen();

		int c = editor_read_key(fd);
		if(c == DEL_KEY || c == BACKSPACE) {
			if(qlen != 0) query[--qlen] = '\0';
		}else if(c == ENTER || c == ESC) {
			if(c == ENTER) {
				editor_parse_command(fd, query);
				return;
			} else {
                                editor_set_status_message("");
                                return;
                        }
		} else if(isprint(c)) {
			if(qlen < BRIC_QUERY_LENGTH) {
				query[qlen++] = c;
				query[qlen] = '\0';
			}
		}
	}
}

// Função que controla todos os comandos existentes do editor
void editor_process_key_press(int fd)
{
        static int quit_times = BRIC_QUIT_TIMES;
        int c = editor_read_key(fd);
        int filerow = Editor.row_offset+Editor.cursor_y;
        int filecol = Editor.column_offset+Editor.cursor_x;
        switch(Editor.mode) {
        case INSERT_MODE:
                switch(c) {
                        case ENTER:
                                editor_insert_newline();
                                break;
                        case CTRL_G:
             			break;
                        case CTRL_Q:
                                //Sair sem salvar o arquivo
				if(Editor.dirty && quit_times) {
                                        editor_set_status_message("WARNING! File has unsaved changes." "Press Ctrl-Q %d more times to quit.", quit_times);
                                        quit_times--;
                                        return;
                                }
                                exit(0);
                                break;
                        case CTRL_S:
                                editor_save();
                                break;
                        case CTRL_Y:
                                editor_yank_row();
                                break;
                        case CTRL_P:
                                editor_paste_row();
                                break;
                        case CTRL_F:
                                editor_find(fd);
                                break;
                        case CTRL_R:
        			 editor_find_replace(fd);
        			 break;
                        case BACKSPACE:
                                editor_delete_char();
                                break;
                        case CTRL_H:
                        case DEL_KEY:
                                editor_move_cursor(ARROW_RIGHT);
                                editor_delete_char();
                                break;
                        case PAGE_UP:
                                editor_move_cursor(PAGE_UP);
                                break;
                        case PAGE_DOWN:
                                editor_move_cursor(PAGE_DOWN);
                                break;
                        case ARROW_UP:
                        case ARROW_DOWN:
                        case ARROW_LEFT:
                        case ARROW_RIGHT:
                                editor_move_cursor(c);
                                break;
                        case CTRL_L: // means refresh screen
                                break;
                        case CTRL_D:
                                if (Editor.mode == INSERT_MODE)
                                {
                                    Editor.mode = SELECTION_MODE;
                                    Editor.selected_base_x = Editor.cursor_x + Editor.column_offset;
                                    Editor.selected_base_y = Editor.cursor_y + Editor.row_offset;
                                    editor_set_status_message(selection_mode_message);
                                }
                                break;
                        case CTRL_C:
                                if (Editor.mode == SELECTION_MODE)
                                {
                                    copy_to_clipboard();
                                }
                                break;
                        case CTRL_V:
                                if (Editor.mode == INSERT_MODE)
                                {
                                    paste_from_clipboard();
                                }
                                break;
                        case ESC:
                                Editor.mode = NORMAL_MODE;
                                editor_set_status_message("Normal mode.");
                                if(filecol != 0)
                                    editor_move_cursor(ARROW_LEFT);
                                break;
                        case HOME_KEY:
                                editor_move_cursor(HOME_KEY);
                                break;
                        case END_KEY:
                        	editor_move_cursor(END_KEY);
                        	break;

                        case TAB:
                            {
                                if(Editor.tab_length < 0)
                                {
                                        editor_insert_char(c);
                                }
                                else
                                {
                                    int i = 0;
                                    while(i < Editor.tab_length){
                                        editor_insert_char(' ');
                                        i++;
                                    }
                                }

                            }
                                break;
                        default:
                                editor_insert_char(c);
                                break;
                }
                break;

        case NORMAL_MODE:
                switch (c)
		{
                        case 'r':
                            if (Editor.prev_char == 'c') editor_copy_row();
                            if (Editor.prev_char == 'y') editor_yank_row();
                            if (Editor.prev_char == 'p') editor_paste_row();
                            if (Editor.prev_char == 'd') editor_delete_row(filerow);
                            break;
                        case 'p':
                            if (Editor.prev_char == 'c') paste_from_clipboard();
                            break;
                        case 'c':
                            if (Editor.prev_char == 'c') Editor.clipboard = "";
                            break;
                        case 'h':
                        case 'j':
                        case 'k':
                        case 'l':
                        case ARROW_LEFT:
                        case ARROW_UP:
                        case ARROW_RIGHT:
                        case ARROW_DOWN:
                                editor_move_cursor(c);
                                break;

                        case ':':
                                enter_command(fd);
                                break;
			// Ativo o Modo de Inserção no local atual do cursor
                        case 'i':
                                Editor.mode = INSERT_MODE;
                                editor_set_status_message("Insert mode.");
                                break;
			// Ativa o Modo de Inserção no começo da linha em que se encontra o cursor
                        case 'I':
				editor_move_cursor(HOME_KEY);
				Editor.mode = INSERT_MODE;
				editor_set_status_message("Insert mode. ");
				break;
			// Cria uma nova linha e muda o cursos para ela
                        case 'o':
				Editor.mode = INSERT_MODE;
				editor_set_status_message("Insert mode.");
				editor_insert_row(filerow + 1, "", 0);
				editor_move_cursor(ARROW_DOWN);
				break;
			// Cria uma nova linha no local atual do cursor
                        case 'O':
				Editor.mode = INSERT_MODE;
				editor_set_status_message("Insert mode. ");
				editor_insert_row(filerow, "", 0);
				break;
			// Pula para a última linha de texto
                        case 'G':
                    		editor_goto(Editor.num_of_rows);
                    		break;
			// Pula para a primeira linha de texto
                        case 'g':
                    		editor_goto(1);
                    		break;
			// Move o cursor para a última coluna (caracter) da linha atual
                        case '$':
				editor_move_cursor(END_KEY);
				break;
			// Move o cursor para a primeira coluna (caracter) da linha atual
                        case '0':
				editor_move_cursor(HOME_KEY);
				break;
			// Ativa o Modo de Inserção um caracter a frente do local atual do cursor
                        case 'a':     
				editor_move_cursor(ARROW_RIGHT);
				Editor.mode = INSERT_MODE;
				editor_set_status_message("Insert mode. ");
				break;
			// Ativa o Modo de Inserção no final da linha atual
                        case 'A':
				Editor.mode = INSERT_MODE;
				editor_set_status_message("Insert mode. ");
				editor_move_cursor(END_KEY);
				editor_move_cursor(ARROW_RIGHT);
				break;
			// Move o cursor para a primeira coluna (caracter) da linha atual
                        case HOME_KEY:
                                editor_move_cursor(HOME_KEY);
                                break;
			// Move o cursor para a última coluna (caracter) da linha atual
                        case END_KEY:
                                editor_move_cursor(END_KEY);
                                break;
                        case PAGE_UP:
                                editor_move_cursor(PAGE_UP);
                                break;
                        case PAGE_DOWN:
                                editor_move_cursor(PAGE_DOWN);
                                break;
                        case CTRL_M:
                                handle_tag_movement(MOVE_AHEAD);
                                break;
                        case CTRL_N:
                                handle_tag_movement(MOVE_BACK);
                                break;
                }
                break;

            case SELECTION_MODE:
                switch(c) 
		{
                    case 'h':
                    case 'j':
                    case 'k':
                    case 'l':
                    case ARROW_UP:
                    case ARROW_DOWN:
                    case ARROW_RIGHT:
                    case ARROW_LEFT:
                        editor_move_cursor(c);
                        break;

                    case ESC:
                        Editor.mode = NORMAL_MODE;
                        break;

                    case CTRL_C:
                        copy_to_clipboard();
                        break;
                }
        }

        Editor.prev_char = c;
        quit_times = BRIC_QUIT_TIMES;
}



int editor_file_was_modified(void)
{
        return Editor.dirty;
}

//Função que inicia o editor com as configurações base
void init_editor(void)
{
        Editor.cursor_x = 0;
        Editor.cursor_y = 0;
        Editor.row_offset = 0;
        Editor.column_offset = 0;
        Editor.num_of_rows = 0;
        Editor.row_head = NULL;
	Editor.row_tail = NULL;
	Editor.current = NULL;
        Editor.dirty = 0;
        Editor.newfile = 0;
        Editor.filename = NULL;
        Editor.syntax = NULL;
	Editor.tab_length = TAB_LENGTH;
        Editor.colours.hl_comment_colour = 33;
        Editor.colours.hl_mlcomment_colour = 33;
        Editor.colours.hl_keyword_cond_colour = 36;
        Editor.colours.hl_keyword_type_colour = 32;
        Editor.colours.hl_keyword_pp_colour = 34;
        Editor.colours.hl_keyword_return_colour = 35;
        Editor.colours.hl_keyword_adapter_colour = 94;
        Editor.colours.hl_keyword_loop_colour = 36;
        Editor.colours.hl_string_colour = 31;
        Editor.colours.hl_number_colour = 34;
        Editor.colours.hl_match_colour = 101;
        Editor.colours.hl_background_colour = 49;
        Editor.colours.hl_default_colour = 37;
        Editor.mode = NORMAL_MODE;

        if(get_window_size(STDIN_FILENO, STDOUT_FILENO, &Editor.screen_rows, &Editor.screen_columns) == -1)
	{
                perror("Unable to query the screen for size (columns / rows)");
                exit(1);
        }

        if (Editor.line_numbers)
        {
            Editor.screen_columns -= line_number_length;
        }

        Editor.prev_char = ' ';
        Editor.screen_rows -= 2; // get room for status bar
}


/*
* Function to load the editor with the infomations
* from an existent file
*/
/* 
* Função que carrega o editor com as informações
* contidas em um arquivo ja existente
*/
void load_config_file(void)
{
    struct passwd *user = getpwuid(getuid());
    char config_file[80];
    char variable_name[60], value[60];
    config_file[0] = 0;
    strcat(config_file, user->pw_dir);
    strcat(config_file, "/.bricrc");

    FILE *config = fopen(config_file, "r");
    if (config == NULL)
    {
        return;
    }

   while (fscanf(config, "set %s %s\n", variable_name, value) == 2)
   {
        if (strcmp(variable_name, "linenumbers") == 0)
        {
            if (strcmp(value, "true") == 0)
            {
                Editor.line_numbers = 1;
            }
            else
            {
                Editor.line_numbers = 0;
            }
        }
        else if (strcmp(variable_name, "indent") == 0)
        {
            if (strcmp(value, "true") == 0)
            {
                Editor.indent = 1;
            }
            else
            {
                Editor.indent = 0;
            }
        }
        else if (strcmp(variable_name, "hl_comment_colour") == 0)
        {
            Editor.colours.hl_comment_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_mlcomment_colour") == 0)
        {
            Editor.colours.hl_mlcomment_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_cond_colour") == 0)
        {
            Editor.colours.hl_keyword_cond_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_type_colour") == 0)
        {
            Editor.colours.hl_keyword_type_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_pp_colour") == 0)
        {
            Editor.colours.hl_keyword_pp_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_return_colour") == 0)
        {
            Editor.colours.hl_keyword_return_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_adapter_colour") == 0)
        {
            Editor.colours.hl_keyword_adapter_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_keyword_loop_colour") == 0)
        {
            Editor.colours.hl_keyword_loop_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_string_colour") == 0)
        {
            Editor.colours.hl_string_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_number_colour") == 0)
        {
            Editor.colours.hl_number_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_match_colour") == 0)
        {
            Editor.colours.hl_match_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_background_colour") == 0)
        {
            Editor.colours.hl_background_colour = atoi(value);
        }
        else if (strcmp(variable_name, "hl_default_colour") == 0)
        {
            Editor.colours.hl_default_colour = atoi(value);
        }
        else if(strcmp(variable_name, "tab_length") == 0)
        {
            Editor.tab_length = atoi(value);
        }

    }

    fclose(config);
}

// Função que fecha o editor de forma correta
void close_editor(void)
{
    free(Editor.filename);

    editing_row *i = Editor.row_head;
    editing_row *prev;

    while (i != NULL)
    {
            prev = i;
            i = i->next;
            if (prev)
            {
                    free(prev->chars);
                    free(prev->rendered_chars);
                    free(prev->hl);
                    free(prev);
            }
    }

    free(Editor.clipboard);
}

// Recebe um nome de arquivo. Inicia o editor com o arquivo aberto
void editor_start(char *filename) {
        free(Editor.filename);
        init_editor();
        Editor.filename = (char*)malloc(sizeof(char) * strlen(filename));
	strcpy(Editor.filename, filename);
        load_config_file();
        editor_select_syntax_highlight(filename);
        editor_open(filename);
        enable_raw_mode(STDIN_FILENO);
        editor_set_status_message(help_message);
}

int main(int argc, char **argv)
{
        signal(SIGWINCH, sigwinch_handler);
        init(&tag_stack);
        int file_arg = -1;

        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] != '-')
            {
                file_arg = i;
            }
        }

        if(file_arg == -1) {
                fprintf(stderr, "Usage: bric <filename>\n");
                exit(1);
        }

        // Setamos a informação acutal do arquivo
	set_current_file(argv[file_arg], &CurrentFile);

	// Verificamos se o arquivo atual está trancado
        if(!is_file_locked(CurrentFile)){
                lock_file(CurrentFile);
        }else{
                fprintf(stderr, "The file has been locked, try to remove the locker!\n");
                return EXIT_FAILURE;
        }

        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                parse_argument(argv[i]);
            }
        }

        editor_start(argv[file_arg]);
        while(1) {
                editor_refresh_screen();
                editor_process_key_press(STDIN_FILENO);
        }

        close_editor();
        return EXIT_SUCCESS;
}
