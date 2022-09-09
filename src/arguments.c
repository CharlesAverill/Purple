/**
 * @file arguments.c
 * @author Charles Averill
 * @brief Command line interface logic
 * @date 08-Sep-2022
 */

#include "arguments.h"

const char* argp_program_version = PROJECT_NAME_AND_VERS;
const char* argp_program_bug_address = "charlesaverill20@gmail.com";
static char doc[] = "The standard compiler for the Purple programming language";
static char args_doc[] = "PROGRAM";

static struct argp_option options[] = {
    {"quiet", 'q', 0, 0, "Don't produce any output", 0},
    {"output", 'o', "FILE", 0, "Path to the generated assembly file", 0},
    {0},
};

error_t parse_opt(int key, char* arg, struct argp_state* state)
{
    purple_args* arguments = state->input;

    int found = 0;

    switch (key) {
    case 'q':
        arguments->quiet = 1;
        break;
    case 'o':
        arguments->filenames[1] = arg;
        break;
    case ARGP_KEY_ARG:
        // Check for too many arguments
        if (state->arg_num > 1) {
            argp_usage(state);
        }

        arguments->filenames[0] = arg;

        break;
    case ARGP_KEY_END:
        // Check for not enough arguments
        if (state->arg_num < 1) {
            argp_usage(state);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

void parse_args(purple_args* args, int argc, char* argv[])
{
    args->quiet = 0;
    args->filenames[1] = "a.s";

    argp_parse(&argp, argc, argv, 0, 0, args);
}
