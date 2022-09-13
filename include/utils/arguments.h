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
 * @struct PurpleArgs
 * @brief Structure containing command line arguments
 */
typedef struct PurpleArgs {
    /**Logging level to use*/
    int logging;
    /**Input filename followed by output ASM filename*/
    char* filenames[2];
} PurpleArgs;

void parse_args(PurpleArgs* args, int argc, char* argv[]);

#endif /* ARGUMENTS_H */
