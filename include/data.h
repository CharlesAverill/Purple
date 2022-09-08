/**
 * @file data.h
 * @author Charles Averill
 * @brief Project-wide variables
 * @date 08-Sep-2022
 */

#ifndef DATA_H
#define DATA_H

#include <stdio.h>

#include "arguments.h"

/**extern_ will be undefined in purple.c, causing purple.c to "own" these variables*/
#ifndef extern_
#define extern_ extern
#endif

/**Current line number of the Scanner*/
extern_ int D_LINE_NUMBER;
/**If defined, this character will be put back into the input stream*/
extern_ char D_PUT_BACK;
/**The pointer to the open filestream for the Scanner*/
extern_ FILE *D_INPUT_FILE;

#define D_MAX_IDENTIFIER_LENGTH 63
/**Buffer to read identifiers into*/
extern_ char D_IDENTIFIER_BUFFER[D_MAX_IDENTIFIER_LENGTH + 1];

/**Activates debug behavior*/
extern_ int D_DEBUG;

/**Command line arguments*/
extern_ purple_args *args;

#endif
