/**
 * @file clang.c
 * @author Charles Averill
 * @brief Functions for interacting with clang
 * @date 13-Sep-2022
 */

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "utils/clang.h"
#include "utils/logging.h"

/**
 * @brief Get the default temporary directory
 * 
 * @return const char* String containing temporary directory name
 */
const char* get_temp_dir(void)
{
    char* tmpdir = NULL;

    if ((tmpdir = getenv("TEMP")) != NULL) {
        (void)0;
    } else if ((tmpdir = getenv("TMP")) != NULL) {
        (void)0;
    } else if ((tmpdir = getenv("TMPDIR")) != NULL) {
        (void)0;
    }

    // Ensure ends with slash
    if (tmpdir && tmpdir[strlen(tmpdir) - 1] != '/') {
        char* formatted = (char*)malloc(strlen(tmpdir + 2));
        formatted[0] = '\0';

        strcat(formatted, tmpdir);
        strcat(formatted, "/");

        return formatted;
    }

    return "/tmp/";
}

/**
 * @brief Create a temporary file for a generator program used to determine compilation information
 */
void create_tmp_generator_program(void)
{
    purple_log(LOG_DEBUG, "Creating generator program file");

    // Setup full paths
    generatorProgramFullPath[0] = '\0';
    generatorProgramLLFullPath[0] = '\0';
    strcat(generatorProgramFullPath, get_temp_dir());
    strcat(generatorProgramFullPath, GENERATOR_PROGRAM_FILENAME);
    strcat(generatorProgramLLFullPath, get_temp_dir());
    strcat(generatorProgramLLFullPath, GENERATOR_PROGRAM_FILENAME_LL);

    // Write to file and close
    FILE* generatorProgramFilePointer = fopen(generatorProgramFullPath, "w");
    if (!generatorProgramFilePointer) {
        fatal(RC_FILE_ERROR, "Failed to open generator program %s", generatorProgramLLFullPath);
    }
    fwrite(GENERATOR_PROGRAM_CONTENTS, GENERATOR_PROGRAM_CONTENTS_LENGTH, 1,
           generatorProgramFilePointer);
    fclose(generatorProgramFilePointer);

    purple_log(LOG_DEBUG, "Compiling generator program to LLVM using clang");

    int clang_status;
    char process_out[256];
    // Generate the clang command
    char cmd[sizeof(args->clang_executable) + 270] = {0};

    strcat(cmd, args->clang_executable);
    strcat(cmd, " -S -emit-llvm -w ");
    strcat(cmd, generatorProgramFullPath);
    strcat(cmd, " -o ");
    strcat(cmd, generatorProgramLLFullPath);

    // Open the process
    purple_log(LOG_DEBUG, "Running clang with \"%s\"", cmd);
    FILE* clang_process = popen(cmd, "r");

    // Let process run
    while (fgets(process_out, 256, clang_process) != NULL) {
        if (args->logging == LOG_DEBUG) {
            printf("%s", process_out);
        }
    }

    // Finish up
    clang_status = pclose(clang_process);
    if (clang_status == -1) {
        purple_log(LOG_ERROR, "clang failed with errno %d while compiling generator program",
                   errno);
    } else if (clang_status != 0) {
        purple_log(LOG_ERROR, "clang exited with return code %d while compiling generator program",
                   clang_status);
    }

    generatorProgramWritten = true;
}

/**
 * @brief Link globals.ll to main LLVM file by copying the contents of each LLVM file into a temporary file, then 
 * copying back to the main LLVM file
 */
void link_globals(void)
{
    // getline buffers
    char* current_llvm_line = NULL;
    size_t llvm_buf_size = (PURPLE_GLOBALS_PLACEHOLDER_LEN + 1);
    char* temp_line = NULL;
    size_t temp_buf_size = 512;

    // Open files
    purple_log(LOG_DEBUG, "Opening files for globals linking");
    FILE* temp_file = tmpfile();
    if (temp_file == NULL) {
        fatal(RC_FILE_ERROR, "Failed to open temporary file");
    }
    FILE* globals_file = fopen(D_LLVM_GLOBALS_FN, "r");
    if (temp_file == NULL) {
        fatal(RC_FILE_ERROR, "Failed to open globals LLVM file");
    }
    FILE* llvm_file = fopen(D_LLVM_FN, "r");
    if (temp_file == NULL) {
        fatal(RC_FILE_ERROR, "Failed to open main LLVM file");
    }

    purple_log(LOG_DEBUG, "Scanning main LLVM file for globals placeholder");
    while (getline(&current_llvm_line, &llvm_buf_size, llvm_file) >= 0) {
        // Check if current line is the placeholder
        if (regex_match(PURPLE_GLOBALS_PLACEHOLDER "\\s*", current_llvm_line, llvm_buf_size + 1,
                        0)) {
            // Copy entire globals file into temp file
            rewind(globals_file);

            while (getline(&temp_line, &temp_buf_size, globals_file) >= 0) {
                fwrite(temp_line, strlen(temp_line), 1, temp_file);
            }
        } else {
            // Copy line into temp file
            fwrite(current_llvm_line, strlen(current_llvm_line), 1, temp_file);
        }
    }

    // Close globals file, close, erase, and reopen LLVM, rewind temp file
    fclose(globals_file);
    fclose(llvm_file);
    llvm_file = fopen(D_LLVM_FN, "w");
    rewind(temp_file);

    // Copy data from temp file to LLVM file
    purple_log(LOG_DEBUG, "Copying data from temp LLVM file to main LLVM file");
    while (getline(&temp_line, &temp_buf_size, temp_file) >= 0) {
        fwrite(temp_line, strlen(temp_line), 1, llvm_file);
    }

    // Close LLVM and temp files
    fclose(llvm_file);
    fclose(temp_file);
}

/**
 * @brief Starts up the clang compiler to compile the generated LLVM-IR into a binary
 * 
 * @param fn Name of file to compile
 */
void clang_compile_llvm(const char* fn)
{
    purple_log(LOG_DEBUG, "Compiling LLVM with clang");

    int clang_status;
    char* process_out;
    size_t process_out_buf_len = 256;
    // Generate the clang command
    char cmd[sizeof(args->clang_executable) + 270] = {0};

    strcat(cmd, args->clang_executable);
    strcat(cmd, " ");
    strcat(cmd, fn);

    // Open the process
    purple_log(LOG_DEBUG, "Running clang with \"%s\"", cmd);
    FILE* clang_process = popen(cmd, "r");

    // Let process run
    while (getline(&process_out, &process_out_buf_len, clang_process) >= 0) {
        if (args->logging == LOG_DEBUG) {
            printf("%s", process_out);
        }
    }

    // Finish up
    clang_status = pclose(clang_process);
    if (clang_status == -1) {
        purple_log(LOG_ERROR, "clang failed with errno %d", errno);
    } else if (clang_status != 0) {
        purple_log(LOG_ERROR, "clang exited with return code %d", clang_status);
    }
}

/**
 * @brief Search through GENERATOR_PROGRAM for the target datalayout
 * 
 * @return char* Pointer to target datalayout string
 */
char* get_target_datalayout(void)
{
    char* out = NULL;

    if (!generatorProgramWritten) {
        create_tmp_generator_program();
    }

    purple_log(LOG_DEBUG, "Retrieving target datalayout");

    char line[1024];
    FILE* generatorProgramFilePointer = fopen(generatorProgramLLFullPath, "r");
    if (!generatorProgramFilePointer) {
        fatal(RC_FILE_ERROR, "Failed to open generator program LLVM file %s",
              generatorProgramLLFullPath);
    }

    while (fgets(line, 1024, generatorProgramFilePointer) != NULL) {
        if ((out = regex_match("target datalayout = \"(.*)\"", line, 1024, 1)) != NULL) {
            break;
        }
    }

    fclose(generatorProgramFilePointer);

    if (out == NULL) {
        fatal(RC_COMPILER_ERROR, "Failed to determine target datalayout");
    }

    return out;
}

/**
 * @brief Get the target triple for the current target
 * 
 * @return char* Pointer to the target triple string
 */
char* get_target_triple(void)
{
    char* process_out = (char*)malloc(64);
    int clang_status;

    purple_log(LOG_DEBUG, "Retrieving target triple");

    // Open the process
    char process_cmd[strlen(args->clang_executable) + 22];
    sprintf(process_cmd, "%s %s", args->clang_executable, "-print-target-triple");
    FILE* clang_process = popen(process_cmd, "r");

    // Let process run
    fgets(process_out, 64, clang_process);

    // Finish up
    clang_status = pclose(clang_process);
    if (clang_status == -1) {
        fatal(RC_ERROR, "clang failed with errno %d while printing target triple", errno);
    } else if (clang_status != 0) {
        fatal(RC_ERROR, "clang exited with return code %d while printing target triple",
              clang_status);
    }

    // Strip newline
    if (process_out[strlen(process_out) - 1] == '\n') {
        process_out[strlen(process_out) - 1] = '\0';
    }

    return process_out;
}

/**
 * @brief Simple regex parser
 * 
 * @param regex Regular expression to compile
 * @param target_str String to search in
 * @param len Maximum length of the output match, if found
 * @param group_index Index of group to match (0 for no group)
 * @return char* Pointer to the match text if match found, else NULL
 */
char* regex_match(const char* regex, char* target_str, int len, int group_index)
{
    regex_t re;
    regmatch_t rm[2];

    if (regcomp(&re, regex, REG_EXTENDED) != 0) {
        fatal(RC_COMPILER_ERROR, "Failed to compile regex %s", regex);
    }

    if (regexec(&re, target_str, 2, rm, 0) == 0) {
        char* out = (char*)malloc(len + 1);
        sprintf(out, "%.*s", (int)(rm[group_index].rm_eo - rm[group_index].rm_so),
                target_str + rm[group_index].rm_so);
        return out;
    }

    return NULL;
}
