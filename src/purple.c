/**
 * @file purple.c
 * @author Charles Averill
 * @brief Compiler entrypoint and setup
 * @date 08-Sep-2022
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define, then undef extern_ to transfer ownership to purple.c
#define extern_
#include "data.h"
#undef extern_

#include "parse.h"
#include "scan.h"
#include "translate/symbol_table.h"
#include "translate/translate.h"
#include "utils/arguments.h"
#include "utils/clang.h"
#include "utils/logging.h"

/**
 * @brief Parse compiler arguments, open input file, and allocate memory 
 * 
 * @param argc 
 * @param argv 
 */
static void init(int argc, char* argv[])
{
    // Argument parsing
    // D_ARGS must be 0-initialized so that default-0 flags don't need to be set
    // in the compiler
    D_ARGS = calloc(1, sizeof(PurpleArgs));
    if (D_ARGS == NULL) {
        fatal(RC_MEMORY_ERROR, "Unable to allocate memory for command line arguments");
    }

    parse_args(D_ARGS, argc, argv);

    if (D_ARGS->from_command_line_argument != NULL) {
        D_INPUT_FN = "argument";
        D_INPUT_FILE = tmpfile();
        fwrite(D_ARGS->from_command_line_argument, strlen(D_ARGS->from_command_line_argument), 1,
               D_INPUT_FILE);
        rewind(D_INPUT_FILE);
    } else {
        D_INPUT_FN = D_ARGS->filenames[0];
        D_INPUT_FILE = fopen(D_INPUT_FN, "r");
        if (D_INPUT_FILE == NULL) {
            fatal(RC_FILE_ERROR, "Unable to open %s: %s", D_INPUT_FN, strerror(errno));
        }
    }

    // Global data
    D_LINE_NUMBER = 1;
    D_CHAR_NUMBER = 1;
    D_PUT_BACK = '\n';

    // Global Token
    scan();

    // Symbol Tables
    D_SYMBOL_TABLE_STACK = new_nonempty_symbol_table_stack();
    D_GLOBAL_SYMBOL_TABLE = D_SYMBOL_TABLE_STACK->top;

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

    link_globals();

    clang_compile_llvm(D_LLVM_FN);

    shutdown();

    return 0;
}
