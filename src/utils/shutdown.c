/**
 * @file shutdown.c
 * @author Charles Averill
 * @brief Functions for shutting down the compiler safely
 * @date 13-Sep-2022
 */

#include <stdlib.h>

#include "data.h"
#include "utils/logging.h"

/**
 * @brief Close any open input/output files
 */
void close_files(void)
{
    purple_log(LOG_DEBUG, "Closing input and output files");

    if (D_INPUT_FILE) {
        fclose(D_INPUT_FILE);
        D_INPUT_FILE = NULL;
    }
    if (D_LLVM_FILE) {
        fclose(D_LLVM_FILE);
        D_LLVM_FILE = NULL;
    }
    if (D_LLVM_GLOBALS_FILE) {
        fclose(D_LLVM_GLOBALS_FILE);
        D_LLVM_GLOBALS_FILE = NULL;
    }
}

/**
 * @brief Tie up any loose ends that may have arisen
 */
void shutdown(void)
{
    purple_log(LOG_DEBUG, "Shutting down");

    close_files();

    if (args) {
        free(args);
    }
}
