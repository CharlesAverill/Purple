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

#include "parse.h"
#include "scan.h"
#include "translate/translate.h"
#include "utils/arguments.h"
#include "utils/clang.h"
#include "utils/logging.h"

static void init(int argc, char* argv[]);

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
    args = malloc(sizeof(PurpleArgs));
    if (args == NULL) {
        fatal(RC_MEMORY_ERROR, "Unable to allocate memory for command line arguments");
    }

    parse_args(args, argc, argv);

    D_INPUT_FN = args->filenames[0];
    D_INPUT_FILE = fopen(D_INPUT_FN, "r");
    if (D_INPUT_FILE == NULL) {
        fatal(RC_FILE_ERROR, "Unable to open %s: %s\n", D_INPUT_FN, strerror(errno));
    }

    // Global Token
    scan(&D_GLOBAL_TOKEN);

    purple_log(LOG_DEBUG, "Compiler initialized");
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

    generate_llvm();

    close_files();

    clang_compile_llvm(D_LLVM_FN);

    shutdown();

    return 0;
}