/**
 * @file declaration.c
 * @author Charles Averill
 * @brief Logic for declaration statements
 * @date 19-Sep-2022
 */

#include "data.h"
#include "parse.h"
#include "translate/llvm.h"
#include "translate/symbol_table.h"

/**
 * @brief Parse a variable declaration statement into LLVM
 */
void variable_declaration(void)
{
    purple_log(LOG_DEBUG, "Parsing variable declaration statement");

    NumberType number_type = match_type();
    if (number_type == -1) {
        fatal(RC_COMPILER_ERROR, "Failed to match number type in variable_declaration");
    }

    match_token(T_IDENTIFIER);
    add_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, D_IDENTIFIER_BUFFER, number_type);
    llvm_declare_global_number_variable(D_IDENTIFIER_BUFFER, number_type);
}