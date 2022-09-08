/**
 * @file
 * @author CharlesAverill
 * @date   12-Oct-2021
 * @brief File headers and global variables for command line arguments
*/

#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <argp.h>
#include <stdlib.h>
#include <string.h>

#include "info.h"

/**
 * @struct purple_args
 * @brief Structure containing command line arguments
 */
typedef struct purple_args {
    int quiet;
    /**Input filename followed by output ASM filename*/
    char *filenames[2]; 
} purple_args;

void parse_args(purple_args *args, int argc, char *argv[]);

#endif
