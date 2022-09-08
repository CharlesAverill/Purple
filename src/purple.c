/**
 * @file purple.c
 * @author Charles Averill
 * @brief Compiler entrypoint
 * @date 08-Sep-2022
 */

#include <stdio.h>

#include "arguments.h"

/**Define, then undef extern_ to transfer ownership to purple.c*/
#define extern_
#include "data.h"
#undef extern_

static void init(int argc, char* argv[]);
void shutdown(int exit_code);

/**
 * Initialize compiler values
 */
static void init(int argc, char *argv[])
{
    // Argument parsing
    args = malloc(sizeof(purple_args));
    if (args == NULL) {
        fprintf(stderr, "Unable to allocate memory for command line arguments");
        shutdown(1);
    }

    parse_args(args, argc, argv);

    D_INPUT_FILE = fopen(args->filenames[0], "r");
    if (D_INPUT_FILE == NULL) {
        fprintf(stderr, "Unable to open %s: %s\n", args->filenames[0], strerror(errno));
        shutdown(1);
    }
}

void shutdown(int exit_code)
{
    fclose(D_INPUT_FILE);
    exit(exit_code);
}

/**
 * @brief Compiler entrypoint
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return int Compiler return code
 */
int main(int argc, char* argv[]) {
    init(argc, argv);
    
    return 0;
}