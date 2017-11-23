#include "locking.h"

/**
 * Function:    set_current_file()
 * 
 * Objective:   Set the informacion relative to CurrentFile global variable
 *              which contains the path to the current opened file,
 *              we could use this information to create a locker file based in
 *              the filename and the path.
 * 
 * Arguments:   filename        <char *>                    Path to the current file.
 *              current_file    <struct __current_file>     Struct with the current file information.
 * 
 * Return:      N/A
 * 
 */
void set_current_file(char *filename, struct __current_file *current_file)
{
    strcpy(current_file->pathname, filename);
    return;
}

/**
 * Function:    get_locker_name()
 * 
 * Objective:   Get the filename used to lock the current file,
 *              we will join the path with the name of the current file,
 *              adding a dot "." to create it as a hidden file.
 * 
 *              (E.J) Current File: /var/www/index.html
 *              (E.J) Locker File:  /var/www/.index.html.lock
 * 
 * Arguments:   current_file    <struct __current_file>     Struct with the current file information.
 * 
 * Return:      <char *>    Name of the locker file.
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
 * Function:    lock_file()
 * 
 * Objective:   Create the locker file used to lock the current file.
 * 
 * Arguments:   current_file    <struct __current_file>     Struct with the current file information.
 * 
 * Return:      N/A
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
 * Function:    unlock_file()
 * 
 * Objective:   Delete the locker file used to lock the current file.
 * 
 * Arguments:   current_file    <struct __current_file>     Struct with the current file information.
 * 
 * Return:      N/A
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
