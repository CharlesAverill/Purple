/**
 * @file arguments.c
 * @author Charles Averill
 * @brief Command line interface logic
 * @date 08-Sep-2022
 */

#include <string.h>

#include "utils/arguments.h"
#include "utils/logging.h"

const char* argp_program_version = PROJECT_NAME_AND_VERS;
const char* argp_program_bug_address = "charlesaverill20@gmail.com";
static char doc[] = "The standard compiler for the Purple programming language";
static char args_doc[] = "PROGRAM";

static struct argp_option options[] = {
    {"logging", 'l', "LEVEL", 0,
     "Level of log statements to print (NONE, DEBUG, INFO, WARNING, ERROR, CRITICAL)", 0},
    {"quiet", 'q', 0, 0, "Equivalent to --logging=NONE", 0},
    {"verbose", 'v', 0, 0, "Equivalent to --logging=DEBUG", 0},
    {"output", 'o', "FILE", 0, "Path to the generated assembly file", 0},
    {0},
};

error_t parse_opt(int key, char* arg, struct argp_state* state)
{
    PurpleArgs* arguments = state->input;

    int found = 0;

    switch (key) {
    case 'q':
        arguments->logging = LOG_NONE;
        break;
    case 'v':
        arguments->logging = LOG_DEBUG;
        break;
    case 'l':
        if (strcmp("NONE", arg)) {
            arguments->logging = LOG_NONE;
        } else if (strcmp("DEBUG", arg)) {
            arguments->logging = LOG_DEBUG;
        } else if (strcmp("INFO", arg)) {
            arguments->logging = LOG_INFO;
        } else if (strcmp("WARNING", arg)) {
            arguments->logging = LOG_WARNING;
        } else if (strcmp("ERROR", arg)) {
            arguments->logging = LOG_ERROR;
        } else if (strcmp("CRITICAL", arg)) {
            arguments->logging = LOG_CRITICAL;
        }
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

void parse_args(PurpleArgs* args, int argc, char* argv[])
{
    args->logging = LOG_INFO;
    args->filenames[1] = "a.s";

    argp_parse(&argp, argc, argv, 0, 0, args);
}
