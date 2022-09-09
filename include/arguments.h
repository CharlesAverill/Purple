/**
 * @file
 * @author CharlesAverill
 * @date   12-Oct-2021
 * @brief Function headers for command line argument parsing
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
    /**Silences all compiler output*/
    int quiet;
    /**Input filename followed by output ASM filename*/
    char* filenames[2];
} purple_args;

void parse_args(purple_args* args, int argc, char* argv[]);

#endif
