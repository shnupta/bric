#include "handling.h"

extern struct editor_config Editor;
extern struct termios orig_termios;


/**
 * Função: disable_raw_mode ()
 *
 * Objetivo: TODO: Escreve este item.
 *
 * Argumentos: fd <int> STDIN File descriptor.
 * termios <struct termios *> Estrutura original dos termios.
 * editor <struct editor_config *> estrutura de configuração do editor.
 *
 * Retorno: N / A
 *
 */
void disable_raw_mode(int fd, struct termios *termios, struct editor_config *editor)
{
    // nao se preocupar em checar o valor de retorno
    if (editor->rawmode)
    {
        tcsetattr(fd, TCSAFLUSH, &(*termios));
        write(fd, "\033[2J\033[H\033[?1049l", 15);
        editor->rawmode = 0;
    }

    return;
}

/**
 * Função: editor_at_exit ()
 *
 * Objetivo: TODO: Escreve este item.
 *
 * Argumentos: N / A
 *
 * Retorno: N / A
 *
 */
void editor_at_exit(void)
{
    disable_raw_mode(STDIN_FILENO, &orig_termios, &Editor);
}

/**
 * Função: enable_raw_mode ()
 *
 * Objetivo: Ler uma chave do terminal colocado no modo raw.
 *
 * Argumentos: fd <int> STDIN File descriptor.
 * termios <struct termios *> TODO: Escreve este item.
 * editor <struct editor_config *> TODO: Escreve este item.
 *
 * Retorno: <int> TODO: Escreve este item.
 *
 */
int enable_raw_mode(int fd, struct termios *termios, struct editor_config *editor)
{
    struct termios raw;

    if (editor->rawmode)
    {
        return 0; // ja habilitado
    }

    if (!isatty(fd))
    {
        goto fatal;
    }

    atexit(editor_at_exit);

    if (tcgetattr(fd, termios) == -1)
    {
        goto fatal;
    }

    raw = *termios; // modifica o modo original

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // output modes - disabilitar pos-processamento
    raw.c_oflag &= ~(OPOST);

    //control modes - set 8 bit chars
    raw.c_cflag |= (CS8);

    //local modes, choing off, canonical off, no extended functions, no signal chars (, etc)
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    //control chars - set return condition: min number of bytes and a timer
    raw.c_cc[VMIN] = 0;  // return each byte, or zero for a timeout
    raw.c_cc[VTIME] = 1; //100ms timeout

    // coloca o terminal em modo raw apos realiar flush
    if (tcsetattr(fd, TCSAFLUSH, &raw) < 0)
    {
        goto fatal;
    }

    editor->rawmode = 1;
    return 0;

fatal:
    errno = ENOTTY;
    return -1;
}


/**
 * Função: editor_read_key ()
 *
 * Objetivo: Ler uma chave do terminal colocado no modo raw.
 *
 * Argumentos: fd <int> STDIN File descriptor.
 *
 * Retorno: <int> TODO: Escreve este item.
 *
 */
int editor_read_key(int fd)
{
    int nread;
    char c, seq[3];
    while ((nread = read(fd, &c, 1)) == 0)
        ;

    if (nread == -1)
    {
        exit(1);
    }

    while (1)
    {
        switch (c)
        {
        case ESC: // sequencia de escape
            // se for um escape, entao timeout
            if (read(fd, seq, 1) == 0)
            {
                return ESC;
            }
            if (read(fd, seq + 1, 1) == 0)
            {
                return ESC;
            }
            // ESC [ sequences
            if (seq[0] == '[')
            {
                if (seq[1] >= '0' && seq[1] <= '9')
                {
                    // escape estenddo, entao ler um byte adicional 
                    if (read(fd, seq + 2, 1) == 0)
                    {
                        return ESC;
                    }
                    if (seq[2] == '~')
                    {
                        switch (seq[1])
                        {
                        case '3':
                            return DEL_KEY;
                        case '5':
                            return PAGE_UP;
                        case '6':
                            return PAGE_DOWN;
                        }
                    }
                }
                else
                {
                    switch (seq[1])
                    {
                    case 'A':
                        return ARROW_UP;
                    case 'B':
                        return ARROW_DOWN;
                    case 'C':
                        return ARROW_RIGHT;
                    case 'D':
                        return ARROW_LEFT;
                    case 'H':
                        return HOME_KEY;
                    case 'F':
                        return END_KEY;
                    }
                }
            }

            // ESC 0 sequences
            else if (seq[0] == 'O')
            {
                switch (seq[1])
                {
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
                }
            }
            break;

        default:
            return c;
        }
    }

    return c;
}

/**
 * Função: get_cursor_pos ()
 *
 * Objetivo: TODO: Escreve este item.
 *
 * Argumentos: ifd <int *> TODO: Escreve este item.
 * ofd <int *> TODO: Escreve este item.
 * rows <int *> TODO: Escreve este item.
 * colunas <int *> TODO: Escreve este item.
 *
 * Retorno: <int> TODO: Escreve este item.
 *
 */
int get_cursor_pos(int ifd, int ofd, int *rows, int *columns)
{
    char buf[32];
    unsigned int i = 0;

    // informa a localizacao do cursor
    if (write(ofd, "\x1b[6n", 4) != 4)
    {
        return -1;
    }

    // le a resposta: ESC [ linhas; R colunas
    while (i < sizeof(buf) - 1)
    {
        if (read(ifd, buf + i, 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';

    // parse
    if (buf[0] != ESC || buf[1] != '[')
    {
        return -1;
    }
    if (sscanf(buf + 2, "%d;%d", rows, columns) != 2)
    {
        return -1;
    }

    return 0;
}

/**
 * Função: get_window_size ()
 *
 * Objetivo: TODO: Escreve este item.
 *
 * Argumentos: ifd <int *> TODO: Escreve este item.
 * ofd <int *> TODO: Escreve este item.
 * rows <int *> TODO: Escreve este item.
 * colunas <int *> TODO: Escreve este item.
 *
 * Retorno: <int> TODO: Escreve este item.
 *
 */
int get_window_size(int ifd, int ofd, int *rows, int *columns)
{
    struct winsize ws;

    if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        // ioctl() falhou, entao buscar no terminal 
        int original_row, original_column, retval;

        // pega a posicao inicial e armazena para posteriormente
        retval = get_cursor_pos(ifd, ofd, &original_row, &original_column);
        if (retval == -1)
        {
            goto failed;
        }

        // vai para a margem inferior direita e pega a posicao
        if (write(ofd, "\x1b[999C\x1b[999B", 12) != 12)
        {
            goto failed;
        }

        retval = get_cursor_pos(ifd, ofd, rows, columns);
        if (retval == -1)
        {
            goto failed;
        }

        // restaura a posicao do cursor
        char seq[32];
        snprintf(seq, 32, "\x1b[%d;%dH", original_row, original_column);
        if (write(ofd, seq, strlen(seq)) == -1)
        {
            // nao conseguiu recuperar a posicao do cursor
        }

        return 0;
    }
    else
    {
        *columns = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }

failed:
    return -1;
}
