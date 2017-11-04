#include "locking.h"

/**
 * Function:    set_current_file()
 * 
 * Objective:   Set the informacion relative to CurrentFile global variable
 *              which contains the path to the current opened file,
 *              we could use this information to create a locker file based in
 *              the filename and the path.
 * 
 * Arguments:   filename    <char *>    Path to the current file.
 * 
 * Return:      N/A
 * 
 */
void set_current_file(char *filename, struct __current_file *current_file)
{
    current_file->path = dirname(filename);
    current_file->name = basename(filename);
    current_file->pathname = filename;

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
 * Arguments:   filename    <char *>    Path to the current file.
 * 
 * Return:      <char *> Name of the locker file.
 * 
 */
char *get_locker_name(char *filename)
{
    // Retrive the path and the name for the current file
    char *dir_filename = dirname(filename);
    char *base_filename = basename(filename);

    // We will store the locker name in the buffer
    char *buffer = NULL;

    // Get the string size
    int length = strlen(dir_filename) + strlen(base_filename) + strlen("./.lock");

    // Set the string and buffer size
    char locker[length];
    buffer = malloc(length);

    // Create the locker name
    strcpy(locker, dir_filename);
    strcat(locker, "/");
    strcat(locker, ".");
    strcat(locker, base_filename);
    strcat(locker, ".lock");

    // Copy the name to buffer
    strcpy(buffer, locker);

    return buffer;
}

/**
 * Function:    lock_file()
 * 
 * Objective:   Create the locker file used to lock the current file.
 * 
 * Arguments:   filename    <char *>    Path to the current file.
 * 
 * Return:      N/A
 * 
 */
void lock_file(char *filename)
{
    char *locker = get_locker_name(filename);
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
 * Arguments:   filename    <char *>    Path to the current file.
 * 
 * Return:      N/A
 * 
 */
void unlock_file(char *filename)
{
    char *locker = get_locker_name(filename);
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
 * Arguments:   filename    <char *>    Path to the current file.
 * 
 * Return:      1 if the current file is locked, 0 otherwise.
 * 
 */
int is_file_locked(char *filename)
{
    char *locker = get_locker_name(filename);
    FILE *locker_file = fopen(locker, "r");

    if (locker_file)
    {
        fclose(locker_file);
        return 1;
    }

    return 0;
}
