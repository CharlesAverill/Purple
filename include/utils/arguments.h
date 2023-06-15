/**
 * @file
 * @author CharlesAverill
 * @date   12-Oct-2021
 * @brief Function headers for command line argument parsing
*/

#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <argp.h>
#include <stdbool.h>

#include "info.h"

/**
 * @struct PurpleArgs
 * @brief Structure containing command line arguments
 */
typedef struct PurpleArgs {
    /**Logging level to use*/
    int logging;
    /**Input filename followed by output ASM filename*/
    char* filenames[3];
    /**Path to clang executable*/
    char* clang_executable;
    /**Program read from stdin*/
    char* from_command_line_argument;

    /**True if constant expressions should be reduced*/
    bool const_expr_reduce;
    /**True if '; function_name' should be printed to llvm file whenver 
     * llvm code is written*/
    bool print_func_annotations;
} PurpleArgs;

void parse_args(PurpleArgs* args, int argc, char* argv[]);
void set_opt_level(PurpleArgs* args, int opt_level);
void help_flags();

// CL Argument Shorthands for argp.h
#define ARGP_HELP_FLAGS 0x100
#define ARGP_LLVM_OUTPUT 0x101
#define FLAGS_START 0x201
#define FCONST_EXPR_REDUCE_CODE 0x202
#define FPRINT_FUNC_ANNOTATIONS 0x203
#define FLAGS_END 0x300

#endif /* ARGUMENTS_H */
