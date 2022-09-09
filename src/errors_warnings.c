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
void fatal(int rc, const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, ERROR_RED "[ERROR] - " ANSI_RESET);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (D_INPUT_FILE) {
        fclose(D_INPUT_FILE);
    }

    exit(rc);
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