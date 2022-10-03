/**
 * @file declaration.c
 * @author Charles Averill
 * @brief Logic for declaration statements
 * @date 19-Sep-2022
 */

#include <string.h>

#include "data.h"
#include "parse.h"
#include "translate/llvm.h"
#include "translate/symbol_table.h"

/**
 * @brief Parse a variable declaration statement into an AST
 */
void variable_declaration(void)
{
    purple_log(LOG_DEBUG, "Parsing variable declaration statement");

    NumberType number_type = match_type();
    if (number_type == -1) {
        fatal(RC_COMPILER_ERROR, "Failed to match number type in variable_declaration");
    }

    match_token(T_IDENTIFIER);
    add_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, D_IDENTIFIER_BUFFER, TYPE_NUMBER(number_type));
    llvm_declare_global_number_variable(D_IDENTIFIER_BUFFER, number_type);
}

/**
 * @brief Parse a function declaration statement into an AST
 * 
 * @return ASTNode* 
 */
ASTNode* function_declaration(void)
{
    ASTNode* out;
    SymbolTableEntry* entry;

    match_token(T_VOID);
    match_token(T_IDENTIFIER);
    position ident_pos = D_GLOBAL_TOKEN.pos;
    ident_pos.char_number -= strlen(D_GLOBAL_TOKEN.value.symbol_name) - 1;

    entry = add_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, D_IDENTIFIER_BUFFER, TYPE_VOID);

    match_token(T_LEFT_PAREN);
    match_token(T_VOID);
    match_token(T_RIGHT_PAREN);

    out = parse_statements();

    out = create_ast_node(T_FUNCTION, out, NULL, NULL, TYPE_VOID, entry->symbol_name);
    add_position_info(out, ident_pos);
    return out;
}
