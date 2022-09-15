/**
 * @file statement.c
 * @author Charles Averill
 * @brief Logic for parsing statements into an AST
 * @date 14-Sep-2022
 */

#include "data.h"
#include "parse.h"
#include "translate/llvm.h"
#include "translate/translate.h"

/**
 * @brief Ensure current token is of a given type, and scan the next token if so
 * 
 * @param type TokenType to ensure
 */
static void match_token(TokenType type)
{
    if (D_GLOBAL_TOKEN.type == type) {
        scan(&D_GLOBAL_TOKEN);
    } else {
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Expected token \"%s\"", tokenStrings[type]);
    }
}

/**
 * @brief Parse a set of statements into ASTs and generate them into LLVM-IR
 */
void parse_statements(void)
{
    purple_log(LOG_DEBUG, "Parsing statements");

    ASTNode* root;
    LLVMValue cg_output;

    do {
        match_token(T_PRINT);

        // Parse printed value
        purple_log(LOG_DEBUG, "Parsing binary expression");
        root = parse_binary_expression(0);

        // Allocate stack space
        purple_log(LOG_DEBUG, "Allocating stack space");
        LLVMStackEntryNode* stack_entries = determine_binary_expression_stack_allocation(root);
        llvm_stack_allocation(stack_entries);
        free_llvm_stack_entry_node_list(stack_entries);

        // Generate LLVM
        purple_log(LOG_DEBUG, "Generating LLVM");
        cg_output = ast_to_llvm(root);
        llvm_print_int(cg_output.value.virtual_register_index);

        loadedRegistersHead = NULL;
        initialize_virtual_registers();

        match_token(T_SEMICOLON);
    } while (D_GLOBAL_TOKEN.type != T_EOF);
}