/**
 * @file logging.h
 * @author Charles Averill
 * @brief Function headers, ANSI defines, and enums for raising internal warnings and errors
 * @date 08-Sep-2022
 */

#ifndef LOGGING_H
#define LOGGING_H

#include "utils/shutdown.h"

/**ANSI code for bold text*/
#define ANSI_BOLD "\033[1m"
/**ANSI code for red text*/
#define ANSI_RED "\033[38:5:196m"
/**ANSI code for orange text*/
#define ANSI_ORANGE "\033[38:5:208m"
/**ANSI code for yellow text*/
#define ANSI_YELLOW "\033[38:5:178m"
/**ANSI code for default text style*/
#define ANSI_RESET "\033[0m"

/**Combination of ANSI codes for red errors*/
#define ERROR_RED ANSI_RED ANSI_BOLD
/**Combination of ANSI codes for orange errors*/
#define ERROR_ORANGE ANSI_ORANGE ANSI_BOLD
/**Combination of ANSI codes for yellow errors*/
#define ERROR_YELLOW ANSI_YELLOW ANSI_BOLD

/**
 * @brief Severity levels of logging statements emitted by the compiler 
 */
typedef enum
{
    LOG_NONE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL
} LogLevel;

/**
 * @brief Maps LogLevels to text and color for displaying
 */
typedef struct {
    /**Level of logging to use*/
    LogLevel level;
    /**Name of logging level*/
    char* name;
    /**ANSI color of logging level*/
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
typedef enum
{
    RC_OK,
    RC_ERROR,
    RC_SYNTAX_ERROR,
    RC_MEMORY_ERROR,
    RC_FILE_ERROR,
    RC_COMPILER_ERROR,
    RC_IDENTIFIER_ERROR,
} ReturnCode;

/**
 * @brief String representation of return codes
 */
static const char* returnCodeStrings[] = {
    "OK",         "ERROR",          "SYNTAX ERROR",    "MEMORY ERROR",
    "FILE ERROR", "COMPILER ERROR", "IDENTIFIER ERROR"};

void fatal(ReturnCode rc, const char* fmt, ...);
void syntax_error(char* fn, int line_number, int char_number, const char* fmt, ...);
void identifier_error(char* fn, int line_number, int char_number, const char* fmt, ...);

void purple_log(LogLevel level, const char* fmt, ...);

#endif /* LOGGING_H */
