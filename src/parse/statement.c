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
void match_token(TokenType type)
{
    if (D_GLOBAL_TOKEN.type == type) {
        scan(&D_GLOBAL_TOKEN);
    } else {
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Expected token \"%s\"", tokenStrings[type]);
    }
}

/**
 * @brief Parse a print statement into LLVM
 */
static void print_statement(void)
{
    ASTNode* root;
    LLVMValue cg_output;

    purple_log(LOG_DEBUG, "Parsing print statement");

    match_token(T_PRINT);

    // Parse printed value
    purple_log(LOG_DEBUG, "Parsing binary expression");
    root = parse_binary_expression(0);

    // Allocate stack space
    purple_log(LOG_DEBUG, "Determining stack space");
    LLVMStackEntryNode* stack_entries = determine_binary_expression_stack_allocation(root);
    purple_log(LOG_DEBUG, "Allocating stack space");
    llvm_stack_allocation(stack_entries);
    purple_log(LOG_DEBUG, "Freeing stack space entries");
    free_llvm_stack_entry_node_list(stack_entries);

    // Generate LLVM
    purple_log(LOG_DEBUG, "Generating LLVM");
    cg_output = ast_to_llvm(
        root, 0); // See TODO in translate.c regarding ast_to_llvm register_number being 0
    llvm_print_int(cg_output.value.virtual_register_index);

    initialize_stack_entry_linked_list(&loadedRegistersHead);
    initialize_stack_entry_linked_list(&freeVirtualRegistersHead);
}

/**
 * @brief Parse an assignment statement into LLVM-IR
 */
static void assignment_statement(void)
{
    ASTNode* left;
    ASTNode* right;
    ASTNode* root;
    SymbolTableEntry* found_entry;

    purple_log(LOG_DEBUG, "Parsing assignment statement");

    // Read identifier
    match_token(T_IDENTIFIER);

    // Ensure identifier name has been declared
    purple_log(LOG_DEBUG, "Searching for identifier name in global symbol table");
    if ((found_entry = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, D_IDENTIFIER_BUFFER)) ==
        NULL) {
        identifier_error(D_INPUT_FN, D_LINE_NUMBER, "Identifier name \"%s\" has not been declared",
                         D_IDENTIFIER_BUFFER);
    }

    // Make a terminal node for the identifier
    right = create_ast_identifier_leaf(T_LVALUE_IDENTIFIER, found_entry->symbol_name);

    match_token(T_ASSIGN);

    // Parse assignment expression
    purple_log(LOG_DEBUG, "Parsing binary expression");
    left = parse_binary_expression(0);

    // Create subtree for assignment statement
    root = create_ast_node(T_ASSIGN, left, NULL, right, 0, NULL);

    // Allocate stack space
    purple_log(LOG_DEBUG, "Determining stack space");
    LLVMStackEntryNode* stack_entries = determine_binary_expression_stack_allocation(left);
    purple_log(LOG_DEBUG, "Allocating stack space");
    llvm_stack_allocation(stack_entries);
    purple_log(LOG_DEBUG, "Freeing stack space entries");
    free_llvm_stack_entry_node_list(stack_entries);

    // Generate LLVM
    purple_log(LOG_DEBUG, "Generating LLVM");
    ast_to_llvm(root, 0); // See TODO above

    initialize_stack_entry_linked_list(&loadedRegistersHead);
    initialize_stack_entry_linked_list(&freeVirtualRegistersHead);
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
        switch (D_GLOBAL_TOKEN.type) {
        case T_PRINT:
            print_statement();
            break;
        case T_INT:
            variable_declaration();
            break;
        case T_IDENTIFIER:
            assignment_statement();
            break;
        case T_EOF:
            return;
        default:
            syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Unexpected token %s",
                         tokenStrings[D_GLOBAL_TOKEN.type]);
            break;
        }

        match_token(T_SEMICOLON);
    } while (D_GLOBAL_TOKEN.type != T_EOF);
}