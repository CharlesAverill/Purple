/**
 * @file logging.h
 * @author Charles Averill
 * @brief Function headers, ANSI defines, and enums for raising internal warnings and errors
 * @date 08-Sep-2022
 */

#ifndef LOGGING_H
#define LOGGING_H

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
 * @brief Severity levels of logging statements emitted by the compiler 
 */
typedef enum { LOG_NONE, LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_CRITICAL } LogLevel;

/**
 * @brief Maps LogLevels to text and color for displaying
 */
typedef struct {
    LogLevel level;
    char* name;
    char* color;
} LogInfo;

/**
 * @brief Collection of LogInfos for standard logging purposes
 */
static const LogInfo logInfoLevels[] = {
    {LOG_NONE, "", ANSI_RESET},          {LOG_DEBUG, "[DEBUG]", ANSI_BOLD},
    {LOG_INFO, "[INFO]", ANSI_BOLD},     {LOG_WARNING, "[WARNING]", ANSI_YELLOW},
    {LOG_ERROR, "[ERROR]", ANSI_ORANGE}, {LOG_CRITICAL, "[CRITICAL]", ANSI_RED}};

/**
 * @brief Return codes used in different scenarios
 */
typedef enum {
    RC_OK,
    RC_ERROR,
    RC_SYNTAX_ERROR,
    RC_MEMORY_ERROR,
    RC_FILE_ERROR,
    RC_COMPILER_ERROR,
} ReturnCode;

/**
 * @brief String representation of return codes
 */
static const char* returnCodeStrings[] = {"OK",           "ERROR",      "SYNTAX ERROR",
                                          "MEMORY ERROR", "FILE ERROR", "COMPILER ERROR"};

void shutdown(void);

void fatal(ReturnCode rc, const char* fmt, ...);
void syntax_error(const char* fn, const int line_number, const char* fmt, ...);

void purple_log(LogLevel level, const char* fmt, ...);

#endif /* LOGGING_H */
