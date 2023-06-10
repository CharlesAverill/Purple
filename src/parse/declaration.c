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

    Number n;
    if (match_type(&n) > 1) {
        fatal(RC_COMPILER_ERROR, "Failed to match variable type in variable_declaration");
    }
    n.pointer_depth++;

    match_token(T_IDENTIFIER);
    add_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, D_IDENTIFIER_BUFFER,
                           TYPE_NUMBER_FROM_NUMBERTYPE_FROM_NUMBER(n));
    llvm_declare_global_number_variable(D_IDENTIFIER_BUFFER, n);
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

    TokenType function_return_type = check_for_type();
    match_token(T_IDENTIFIER);

    D_CURRENT_FUNCTION_BUFFER[0] = '\0';
    strcpy(D_CURRENT_FUNCTION_BUFFER, D_GLOBAL_TOKEN.value.symbol_name);

    position ident_pos = D_GLOBAL_TOKEN.pos;
    ident_pos.char_number -= strlen(D_GLOBAL_TOKEN.value.symbol_name) - 1;

    Type function_type = TYPE_FUNCTION(function_return_type, 0, 0);
    entry = add_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, D_IDENTIFIER_BUFFER, TYPE_VOID);

    match_token(T_LEFT_PAREN);

    unsigned long long int parameters_size = 32;
    int num_inputs = 0;
    FunctionParameter* parameters =
        (FunctionParameter*)malloc(sizeof(FunctionParameter) * parameters_size);
    while (D_GLOBAL_TOKEN.type != T_RIGHT_PAREN) {
        Number param_type;
        if (match_type(&param_type) == 1) {
            break;
        }
        param_type.pointer_depth++;

        parameters[num_inputs].parameter_type = param_type;
        if (num_inputs >= parameters_size) {
            parameters_size *= 2;
            parameters = (FunctionParameter*)realloc(parameters, parameters_size);
        }

        num_inputs++;
        function_type.value.function.num_parameters++;

        match_token(T_IDENTIFIER);
        strcpy(parameters[num_inputs - 1].parameter_name, D_IDENTIFIER_BUFFER);

        // TODO : Locals
        add_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, D_IDENTIFIER_BUFFER,
                               TYPE_NUMBER_FROM_NUMBERTYPE_FROM_NUMBER(param_type));
        llvm_declare_global_number_variable(D_IDENTIFIER_BUFFER, param_type);
    }

    function_type.value.function.parameters = parameters;
    entry->type = function_type;

    match_token(T_RIGHT_PAREN);

    out = parse_statements();

    out =
        create_ast_node(T_FUNCTION_DECLARATION, out, NULL, NULL, function_type, entry->symbol_name);
    add_position_info(out, ident_pos);
    return out;
}
