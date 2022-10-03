/**
 * @file logging.c
 * @author Charles Averill
 * @brief Warnings and fatal error handling
 * @date 08-Sep-2022
 */

#include <stdarg.h>
#include <stdlib.h>

#include "data.h"
#include "utils/arguments.h"
#include "utils/logging.h"

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

    shutdown();

    exit(rc);
}

/**
 * @brief Raises a fatal syntax error
 * 
 * @param fn Filename in which syntax error occurs
 * @param line_number Line number on which syntax error occurs
 * @param char_number Character numbre on which syntax error occurs
 * @param fmt Format string for details printed before fatal error
 * @param ... Varargs for details printed before fatal error
 */
void syntax_error(char* fn, int line_number, int char_number, const char* fmt, ...)
{
    va_list func_args;

    if (fn == NULL) {
        fn = D_INPUT_FN;
    }
    if (line_number == 0) {
        line_number = D_LINE_NUMBER;
    }
    if (char_number == 0) {
        char_number = D_CHAR_NUMBER;
    }

    // Print fence for error distinguishing
    fprintf(stderr, "----------------------------------------\n");

    // Print details of error
    va_start(func_args, fmt);
    vfprintf(stderr, fmt, func_args);
    va_end(func_args);

    fprintf(stderr, "\n");

    fatal(RC_SYNTAX_ERROR, "%s:%d:%d", fn, line_number, char_number);
}

/**
 * @brief Raises a fatal identifier error
 * 
 * @param fn Filename in which identifier error occurs
 * @param line_number Line number on which identifier error occurs
 * @param char_number Character numbre on which syntax error occurs
 * @param fmt Format string for details printed before fatal error
 * @param ... Varargs for details printed before fatal error
 */
void identifier_error(char* fn, int line_number, int char_number, const char* fmt, ...)
{
    va_list func_args;

    if (fn == NULL) {
        fn = D_INPUT_FN;
    }
    if (line_number == 0) {
        line_number = D_LINE_NUMBER;
    }
    if (char_number == 0) {
        char_number = D_CHAR_NUMBER;
    }

    // Print fence for error distinguishing
    fprintf(stderr, "----------------------------------------\n");

    // Print details of error
    va_start(func_args, fmt);
    vfprintf(stderr, fmt, func_args);
    va_end(func_args);

    fprintf(stderr, "\n");

    fatal(RC_IDENTIFIER_ERROR, "%s:%d:%d", fn, line_number, char_number);
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

    if (args) {
        if (args->logging == LOG_NONE || args->logging > level) {
            return;
        }

        if (args->logging > LOG_INFO) {
            output_stream = stderr;
        } else {
            output_stream = stdout;
        }
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
