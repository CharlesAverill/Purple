/**
 * @file data.h
 * @author Charles Averill
 * @brief Project-wide variables
 * @date 08-Sep-2022
 */

#ifndef DATA_H
#define DATA_H

#include <stdio.h>

#include "scan.h"
#include "translate/symbol_table.h"
#include "utils/arguments.h"

/**extern_ will be undefined in purple.c, causing purple.c to "own" these variables*/
#ifndef extern_
#define extern_ extern
#endif

/**Current line number of the Scanner*/
extern_ int D_LINE_NUMBER;
/**If defined, this character will be put back into the input stream*/
extern_ char D_PUT_BACK;
/**The file pointer to the open filestream for the Scanner*/
extern_ FILE* D_INPUT_FILE;
/**The file pointer to the open filestream for the output LLVM-IR file*/
extern_ FILE* D_LLVM_FILE;
/**The file pointer to the open filestream for the output LLVM-IR Global Variables file*/
extern_ FILE* D_LLVM_GLOBALS_FILE;
/**Filename corresponding to D_INPUT_FILE*/
extern_ char* D_INPUT_FN;
/**Filename corresponding to D_LLVM_FILE*/
extern_ char* D_LLVM_FN;
/**Filename corresponding to D_LLVM_GLOBALS_FILE*/
extern_ char* D_LLVM_GLOBALS_FN;
/**Current number of the latest-used LLVM virtual register within a function*/
extern_ unsigned long long int D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER;
/**Current label index*/
extern_ unsigned long long int D_LABEL_INDEX;
/**Whether or not the current function has printed its preamble to LLVM_FILE yet*/
extern_ bool D_CURRENT_FUNCTION_PREAMBLE_PRINTED;

/**Buffer to read identifiers into*/
extern_ char D_IDENTIFIER_BUFFER[D_MAX_IDENTIFIER_LENGTH + 1];

/**Activates debug behavior*/
extern_ int D_DEBUG;

/**Command line arguments*/
extern_ PurpleArgs* args;

/**Most recently-parsed token*/
extern_ struct Token D_GLOBAL_TOKEN;

/**Symbol Table Stack with the Global Symbol Table as its bottom*/
extern_ SymbolTableStack* D_SYMBOL_TABLE_STACK;
/**Global Symbol Table (pointer to bottom of D_SYMBOL_TABLE_STACK)*/
extern_ SymbolTable* D_GLOBAL_SYMBOL_TABLE;

#endif /* DATA_H */
