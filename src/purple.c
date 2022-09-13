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

static void compile_llvm() {
    purple_log(LOG_DEBUG, "Compiling LLVM with clang");

    int clang_status;
    char process_out[256];
    // Generate the clang command
    char cmd[270] = {0};

    strcat(cmd, "clang ");
    strcat(cmd, D_LLVM_FN);
    
    // Open the process
    purple_log(LOG_DEBUG, "Running clang with \"%s\"", cmd);
    FILE* clang_process = popen(cmd, "r");

    // Let process run
    while (fgets(process_out, 256, clang_process) != NULL) {
        if(args->logging <= LOG_DEBUG) {
            printf("%s", process_out);
        }
    }

    // Finish up
    clang_status = pclose(clang_process);
    if(clang_status == -1) {
        purple_log(LOG_ERROR, "clang failed with errno %d", errno);
    } 
    /*
    else if(clang_status != 0){
        purple_log(LOG_ERROR, "clang exited with return code %d", clang_status);
    }
    */
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
    struct ASTNode* n;

    init(argc, argv);

    purple_log(LOG_DEBUG, "Parsing binary expression");
    n = parse_binary_expression(0);

    generate_llvm(n);

    close_files();

    compile_llvm();

    shutdown();

    return 0;
}