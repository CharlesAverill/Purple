/**
 * @file errors_warnings.c
 * @author Charles Averill
 * @brief Warnings and fatal error handling
 * @date 08-Sep-2022
 */

#include "errors_warnings.h"

/**
 * @brief Raises a fatal error that will exit the compiler
 * 
 * @param rc Return code to exit with
 * @param fmt Format string for printed error
 * @param ... Varargs for printed error
 */
void fatal(ReturnCode rc, const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, "%s%s%s", ERROR_RED "[", returnCodeStrings[rc], "] - " ANSI_RESET);
    vfprintf(stderr, fmt, args);
    va_end(args);

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
    va_list args;

    // Print fence for error distinguishing
    fprintf(stderr, "----------------------------------------\n");

    // Print details of error
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");

    fatal(RC_SYNTAX_ERROR, "%s:%d", fn, line_number);
}

/**
 * @brief Raises a non-fatal warning
 * 
 * @param level Severity of warning
 * @param fmt Format string for printed warning
 * @param ... Varargs for printed warning
 */
void warning(WarningType level, const char* fmt, ...)
{
    va_list args;
    char* warning_color;

    va_start(args, fmt);
    switch (level) {
    case WARNING_LOW:
        warning_color = ERROR_YELLOW;
        break;
    case WARNING_MED:
        warning_color = ERROR_ORANGE;
        break;
    case WARNING_HIGH:
        warning_color = ERROR_RED;
        break;
    }
    fprintf(stderr, "%s%s", warning_color, "[WARNING] - " ANSI_RESET);
    vfprintf(stderr, fmt, args);
    va_end(args);
}