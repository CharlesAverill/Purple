/**
 * @file arguments.c
 * @author Charles Averill
 * @brief Command line interface logic
 * @date 08-Sep-2022
 */

#include <stdlib.h>
#include <string.h>

#include "info.h"
#include "utils/arguments.h"
#include "utils/formatting.h"
#include "utils/logging.h"

const char* argp_program_version = PROJECT_NAME_AND_VERS;
const char* argp_program_bug_address = "charlesaverill20@gmail.com";
static char doc[] = "The standard compiler for the Purple programming language";
static char args_doc[] = "PROGRAM";

static struct argp_option options[] = {
    {0, 0, 0, 0, "Informational Options:", 1},
    {"logging", 'l', "LEVEL", 0,
     "Level of log statements to print (NONE, DEBUG, INFO, WARNING, ERROR, CRITICAL)", 0},
    {"quiet", 'q', 0, 0, "Equivalent to --logging=NONE", 0},
    {"verbose", 'v', 0, 0, "Equivalent to --logging=DEBUG", 0},
    {"help-flags", ARGP_HELP_FLAGS, 0, 0, "Show help information for various flags used in Purple",
     0},
    {0, 0, 0, 0, "Compiler Options:", 2},
    {"clang-executable", 'g', "PATH", 0,
     "Alternate path to clang executable (default is \"" DEFAULT_CLANG_EXECUTABLE_PATH "\"", 0},
    {"cmd", 'c', "PROGRAM", OPTION_HIDDEN, "Program passed in as a string", 0},
    {"llvm-output", ARGP_LLVM_OUTPUT, "FILE", 0, "Path to the generated LLVM file", 0},
    {"output", 'o', "FILE", 0, "Path to compiled binary", 0},
    {"opt", 'O', "OPTLEVEL", 0, "Level of optimization to enable (0-3)"},
    {"fconst-expr-reduce", FCONST_EXPR_REDUCE_CODE, 0, OPTION_HIDDEN,
     "Reduces constant expressions at compile-time", 0},
    {"fprint-func-annotations", FPRINT_FUNC_ANNOTATIONS, 0, OPTION_HIDDEN,
     "When generating llvm, prints a comment containing which function in the compiler is printing",
     0},
    {0, 0, 0, 0, "Generic Options:", -1},
    {0},
};

error_t parse_opt(int key, char* arg, struct argp_state* state)
{
    PurpleArgs* arguments = state->input;

    int found = 0;

    switch (key) {
    case 'c':
        arguments->from_command_line_argument = arg;
        break;
    case 'g':
        arguments->clang_executable = arg;
        break;
    case 'l':
        if (!strcmp("NONE", arg)) {
            arguments->logging = LOG_NONE;
        } else if (!strcmp("DEBUG", arg)) {
            arguments->logging = LOG_DEBUG;
        } else if (!strcmp("INFO", arg)) {
            arguments->logging = LOG_INFO;
        } else if (!strcmp("WARNING", arg)) {
            arguments->logging = LOG_WARNING;
        } else if (!strcmp("ERROR", arg)) {
            arguments->logging = LOG_ERROR;
        } else if (!strcmp("CRITICAL", arg)) {
            arguments->logging = LOG_CRITICAL;
        } else {
            argp_usage(state);
        }
        break;
    case ARGP_LLVM_OUTPUT:
        arguments->filenames[1] = arg;
        break;
    case 'o':
        arguments->filenames[2] = arg;
        break;
    case 'q':
        arguments->logging = LOG_NONE;
        break;
    case 'v':
        arguments->logging = LOG_DEBUG;
        break;
    case 'O':
        if (!arg) {
            fatal(RC_ARG_ERROR, "Expected optimization level");
        }
        set_opt_level(arguments, atoi(arg));
        break;
    case ARGP_HELP_FLAGS:
        help_flags();
        exit(0);
    case FCONST_EXPR_REDUCE_CODE:
        arguments->const_expr_reduce = true;
        break;
    case FPRINT_FUNC_ANNOTATIONS:
        arguments->print_func_annotations = true;
        break;
    case ARGP_KEY_ARG:
        // Check for too many arguments
        if (state->arg_num > 1) {
            argp_usage(state);
        }

        if (arguments->from_command_line_argument != NULL) {
            argp_usage(state);
        }

        arguments->filenames[0] = arg;

        break;
    case ARGP_KEY_END:
        // Check for not enough arguments
        if (state->arg_num < 1 && arguments->from_command_line_argument == NULL) {
            argp_usage(state);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

/**
 * @brief Parses command-line arguments via argp
 */
void parse_args(PurpleArgs* args, int argc, char* argv[])
{
    args->logging = LOG_INFO;
    args->filenames[1] = "a.ll";
    args->filenames[2] = "a.out";
    args->clang_executable = "/usr/bin/clang";
    args->from_command_line_argument = NULL;

    argp_parse(&argp, argc, argv, 0, 0, args);
}

/**
 * @brief Set optimization flags based on a given opt level
 * 
 * @param args      PurpleArgs struct to set flags in
 * @param opt_level Level of optimization to set
 */
void set_opt_level(PurpleArgs* args, int opt_level)
{
    switch (opt_level) {
    // On-purpose fallthrough so that higher levels automatically
    // set the flags from lower levels
    case 3:
    case 2:
    case 1:
        args->const_expr_reduce = true;
        break;
    default:
        break;
    }
}

void help_flags()
{
    printf("Compiler flags:\n");
    for (int i = 0; i < sizeof options / sizeof options[0]; i++) {
        struct argp_option opt = options[i];
        if (opt.key > FLAGS_START && opt.key < FLAGS_END) {
            printf("  --%-30s%s\n", opt.name, opt.doc);
        }
    }
}
