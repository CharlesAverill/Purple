/**
 * @file logging.c
 * @author Charles Averill
 * @brief Warnings and fatal error handling
 * @date 08-Sep-2022
 */

#include "utils/logging.h"
#include "utils/arguments.h"

/**
 * @brief Raises a fatal error that will exit the compiler
 * 
 * @param rc Return code to exit with
 * @param fmt Format string for printed error
 * @param ... Varargs for printed error
 */
void fatal(ReturnCode rc, const char* fmt, ...)
{
    va_list func_args;

    va_start(func_args, fmt);
    fprintf(stderr, "%s%s%s", ERROR_RED "[", returnCodeStrings[rc], "] - " ANSI_RESET);
    vfprintf(stderr, fmt, func_args);
    va_end(func_args);

    // Print fence for error distinguishing
    fprintf(stderr, "\n----------------------------------------\n");

    if (D_INPUT_FILE) {
        fclose(D_INPUT_FILE);
    }

    exit(rc);
}

/**
 * @brief Raises a fatal syntax error
 * 
 * @param fn Filename in which syntax error occurs
 * @param line_number Line number on which syntax error occurs
 * @param fmt Format string for details printed before fatal error
 * @param ... Varargs for details printed before fatal error
 */
void syntax_error(const char* fn, const int line_number, const char* fmt, ...)
{
    va_list func_args;

    // Print fence for error distinguishing
    fprintf(stderr, "----------------------------------------\n");

    // Print details of error
    va_start(func_args, fmt);
    vfprintf(stderr, fmt, func_args);
    va_end(func_args);

    fprintf(stderr, "\n");

    fatal(RC_SYNTAX_ERROR, "%s:%d", fn, line_number);
}

/**
 * @brief Raises a non-fatal logging statement
 * 
 * @param level Severity of statement
 * @param fmt Format string for printed statement
 * @param ... Varargs for printed statement
 */
void purple_log(LogLevel level, const char* fmt, ...)
{
    va_list func_args;
    FILE* output_stream;

    if (args->logging == LOG_NONE || args->logging > level) {
        return;
    }

    if (args->logging > LOG_INFO) {
        output_stream = stderr;
    } else {
        output_stream = stdout;
    }

    va_start(func_args, fmt);
    fprintf(output_stream, "LOG:%s%s%s", logInfoLevels[level].color, logInfoLevels[level].name,
            " - " ANSI_RESET);
    vfprintf(output_stream, fmt, func_args);
    fprintf(output_stream, "\n");
    va_end(func_args);
}