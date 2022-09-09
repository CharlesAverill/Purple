/**
 * @file purple.c
 * @author Charles Averill
 * @brief Compiler entrypoint and setup
 * @date 08-Sep-2022
 */

#include <stdio.h>

// Define, then undef extern_ to transfer ownership to purple.c
#define extern_
#include "data.h"
#undef extern_

#include "arguments.h"
#include "errors_warnings.h"
#include "scan.h"

static void init(int argc, char* argv[]);
void shutdown(void);

/**
 * @brief Parse compiler arguments, open input file, and allocate memory 
 * 
 * @param argc 
 * @param argv 
 */
static void init(int argc, char* argv[])
{
    // Global data
    D_LINE_NUMBER = 1;
    D_PUT_BACK = '\n';

    // Argument parsing
    args = malloc(sizeof(purple_args));
    if (args == NULL) {
        fatal(1, "Unable to allocate memory for command line arguments");
    }

    parse_args(args, argc, argv);

    D_INPUT_FILE = fopen(args->filenames[0], "r");
    if (D_INPUT_FILE == NULL) {
        fatal(1, "Unable to open %s: %s\n", args->filenames[0], strerror(errno));
    }
}

/**
 * @brief Tie up any loose ends that may have arisen
 */
void shutdown(void)
{
    if (D_INPUT_FILE) {
        fclose(D_INPUT_FILE);
    }
}

/**
 * @brief Compiler entrypoint
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return int Compiler return code
 */
int main(int argc, char* argv[])
{
    init(argc, argv);

    Token T;

    while (scan(&T)) {
        printf("Token %s", token_strings[T.type]);
        if (T.type == T_INTEGER_LITERAL) {
            printf(", value %d", T.value);
        }
        printf("\n");
    }

    shutdown();

    return 0;
}