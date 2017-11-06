/**
 * File:        locking.h
 * 
 * Description: This file contents the functions used to handle the file locking, 
 *              based in the current filename.
 * 
 * Author: Ivan Botero <ivan.botero@protonmail.ch>
 */

#ifndef LOCKING_H
#define LOCKING_H

/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

/* Structures */
struct __current_file
{
    char pathname[512];
};

/* Functions */
void set_current_file(char *filename, struct __current_file *current_file);
char *get_locker_name(struct __current_file current_file);

void lock_file(struct __current_file current_file);
void unlock_file(struct __current_file current_file);

int is_file_locked(struct __current_file current_file);

#endif
