/**
 * @file errors_warnings.h
 * @author Charles Averill
 * @brief Function headers, ANSI defines, and enums for raising internal warnings and errors
 * @date 08-Sep-2022
 */

#ifndef ERRORS_WARNINGS_H
#define ERRORS_WARNINGS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "data.h"

#define ANSI_BOLD "\033[1m"
#define ANSI_RED "\033[38:5:196m"
#define ANSI_ORANGE "\033[38:5:208m"
#define ANSI_YELLOW "\033[38:5:178m"
#define ANSI_RESET "\033[0m"

#define ERROR_RED ANSI_RED ANSI_BOLD
#define ERROR_ORANGE ANSI_ORANGE ANSI_BOLD
#define ERROR_YELLOW ANSI_YELLOW ANSI_BOLD

/**
 * @brief Severity levels of warnings raised by the compiler 
 */
typedef enum { WARNING_LOW, WARNING_MED, WARNING_HIGH } WarningType;

void fatal(int rc, const char* fmt, ...);
void warning(WarningType level, const char* fmt, ...);

#endif /* ERRORS_WARNINGS_H */
