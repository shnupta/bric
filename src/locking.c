#include "locking.h"

/**
 * Funcao:    set_current_file()
 * 
 * Objetivo:   
 *              Definir a informação relativa à variável global CurrentFile
 *              que contém o caminho para o arquivo aberto atual,
 *              poderíamos usar esta informação para criar um arquivo de armário baseado em
 *              o nome do arquivo e o caminho.
 * 
 * Argumentos:   nome do arquivo    <char *>    Caminho para o arquivo.
 * 
 * Retorno:      N/A
 * 
 */
void set_current_file(char *filename, struct __current_file *current_file)
{
    strcpy(current_file->pathname, filename);
    return;
}

/**
 * Funcao:  get_locker_name ()
 *
 * Objetivo: 
 *          Obter o nome do arquivo usado para bloquear o arquivo atual,
 *          vamos nos juntar ao caminho com o nome do arquivo atual,
 *          dicionando um ponto "." para criá-lo como um arquivo oculto.
 *
 * (E.J) Arquivo atual: /var/www/index.html
 * (E.J) Locker File: /var/www/.index.html.lock
 *
 * Argumentos: filename <char *> Caminho para o arquivo atual.
 *
 * Retorno: <char *> Nome do arquivo do armário.
 *
 */
char *get_locker_name(struct __current_file current_file)
{
    char locker[512];
    char *buffer = malloc(512);

    sprintf(locker, "%s/.%s.lock", dirname(current_file.pathname), basename(current_file.pathname));
    strcpy(buffer, locker);

    return buffer;
}

/**
 * Função: lock_file ()
 *
 * Objetivo: criar o arquivo de bloqueio usado para bloquear o arquivo atual.
 *
 * Argumentos: filename <char *> Caminho para o arquivo atual.
 *
 * Retorno: N / A
 *
 */
void lock_file(struct __current_file current_file)
{
    char *locker = get_locker_name(current_file);
    FILE *locker_file = fopen(locker, "w");

    if (!locker_file)
    {
        perror("Could not created locker file");
    }
    else
    {
        fclose(locker_file);
    }

    return;
}

/**
 * Função: is_file_locked ()
 *
 * Objetivo: verificar se o arquivo atual está bloqueado.
 * Verifica se o arquivo do armário existe.
 *
 * Argumentos: filename <char *> Caminho para o arquivo atual.
 *
 * Retorno: 1 se o arquivo atual estiver bloqueado, 0 caso contrário.
 *
 */
void unlock_file(struct __current_file current_file)
{
    char *locker = get_locker_name(current_file);
    FILE *locker_file = fopen(locker, "r");

    if (locker_file)
    {
        fclose(locker_file);
        remove(locker);
    }

    return;
}

/**
 * Function:    is_file_locked()
 * 
 * Objective:   Check if the current file is locked. 
 *              It checks if the locker file exists.
 * 
 * Arguments:   current_file    <struct __current_file>     Struct with the current file information.
 * 
 * Return:      <int>           If the current file is locked 1, otherwise 0.
 * 
 */
int is_file_locked(struct __current_file current_file)
{
    char *locker = get_locker_name(current_file);
    FILE *locker_file = fopen(locker, "r");

    if (locker_file)
    {
        fclose(locker_file);
        return 1;
    }

    return 0;
}
