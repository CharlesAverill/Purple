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
 * @brief Ensure current token is a type token, and scan the next token if so
 * 
 * @return NumberType Type of variable
 */
NumberType match_type(void)
{
    TokenType ttype = D_GLOBAL_TOKEN.type;
    if (TOKENTYPE_IS_TYPE(ttype)) {
        scan(&D_GLOBAL_TOKEN);
    } else {
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Expected type");
    }

    return token_type_to_number_type(ttype);
}

/**
 * @brief Parse a print statement into an AST
 * 
 * @return ASTNode* AST for print statement
 */
static ASTNode* print_statement(void)
{
    ASTNode* root;
    LLVMValue cg_output;

    purple_log(LOG_DEBUG, "Parsing print statement");

    match_token(T_PRINT);

    // Parse printed value
    purple_log(LOG_DEBUG, "Parsing binary expression for print statement");
    root = parse_binary_expression(0);

    // Allocate stack space
    purple_log(LOG_DEBUG, "Determining stack space");
    LLVMStackEntryNode* stack_entries = determine_binary_expression_stack_allocation(root);
    purple_log(LOG_DEBUG, "Allocating stack space");
    llvm_stack_allocation(stack_entries);
    purple_log(LOG_DEBUG, "Freeing stack space entries");
    free_llvm_stack_entry_node_list(stack_entries);

    root = create_unary_ast_node(T_PRINT, root, 0);

    return root;
}

/**
 * @brief Parse an assignment statement into an AST
 * 
 * @return ASTNode* AST for assignment
 */
static ASTNode* assignment_statement(void)
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
    purple_log(LOG_DEBUG, "Parsing binary expression for assign statement");
    left = parse_binary_expression(0);

    // Create subtree for assignment statement
    root = create_ast_node(T_ASSIGN, left, NULL, right, 0, NULL);

    return root;
}

/**
 * @brief Parse an if statement into an AST
 * 
 * @return ASTNode* AST for if statement
 */
static ASTNode* if_statement(void)
{
    ASTNode* condition = NULL;
    ASTNode* true_branch = NULL;
    ASTNode* false_branch = NULL;

    purple_log(LOG_DEBUG, "Parsing if statement");

    match_token(T_IF);
    match_token(T_LEFT_PAREN);

    condition = parse_binary_expression(0);

    if (!TOKENTYPE_IS_COMPARATOR(condition->ttype)) {
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Condition clauses must use a comparison operator");
    }

    match_token(T_RIGHT_PAREN);

    true_branch = parse_statements();

    if (D_GLOBAL_TOKEN.type == T_ELSE) {
        match_token(T_ELSE);
        false_branch = parse_statements();
    }

    return create_ast_node(T_IF, condition, true_branch, false_branch, 0, NULL);
}

/**
 * @brief Parse a while statement into an AST
 * 
 * @return ASTNode* AST for while statement
 */
static ASTNode* while_statement(void)
{
    ASTNode* condition = NULL;
    ASTNode* body = NULL;
    ASTNode* else_body = NULL;

    purple_log(LOG_DEBUG, "Parsing while statement");

    match_token(T_WHILE);
    match_token(T_LEFT_PAREN);

    condition = parse_binary_expression(0);

    if (!TOKENTYPE_IS_COMPARATOR(condition->ttype)) {
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Condition clauses must use a comparison operator");
    }

    match_token(T_RIGHT_PAREN);

    body = parse_statements();

    if (D_GLOBAL_TOKEN.type == T_ELSE) {
        purple_log(LOG_DEBUG, "Encountered while-else statement");

        match_token(T_ELSE);
        else_body = parse_statements();
    }

    return create_ast_node(T_WHILE, condition, body, else_body, 0, NULL);
}

/**
 * @brief Parse a for statement into an AST
 * 
 * @return ASTNode* AST for for statement
 */
static ASTNode* for_statement(void)
{
    ASTNode* for_preamble;
    ASTNode* condition;
    ASTNode* for_postamble;
    ASTNode* body;
    ASTNode* else_body = NULL;
    ASTNode* out;

    purple_log(LOG_DEBUG, "Parsing for statement");

    match_token(T_FOR);
    match_token(T_LEFT_PAREN);

    for_preamble = assignment_statement();

    match_token(T_SEMICOLON);

    condition = parse_binary_expression(0);
    if (!TOKENTYPE_IS_COMPARATOR(condition->ttype)) {
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Condition clauses must use a comparison operator");
    }

    match_token(T_SEMICOLON);

    for_postamble = assignment_statement();

    match_token(T_RIGHT_PAREN);

    body = parse_statements();

    if (D_GLOBAL_TOKEN.type == T_ELSE) {
        purple_log(LOG_DEBUG, "Encountered for-else statement");

        match_token(T_ELSE);
        else_body = parse_statements();
    }

    out = create_ast_node(T_AST_GLUE, for_postamble, NULL, else_body, 0, NULL);
    out = create_ast_node(T_WHILE, condition, body, out, 0, NULL);
    out = create_ast_node(T_AST_GLUE, for_preamble, NULL, out, 0, NULL);
    return out;
}

/**
 * @brief Parse a set of statements into ASTs and generate them into LLVM-IR
 * 
 * @return AST for a group of statements
 */
ASTNode* parse_statements(void)
{
    purple_log(LOG_DEBUG, "Parsing statements");

    ASTNode* left = NULL;
    ASTNode* root;
    LLVMValue cg_output;

    match_token(T_LEFT_BRACE);

    while (true) {
        bool return_left = false;
        bool match_semicolon = true;

        if (TOKENTYPE_IS_TYPE(D_GLOBAL_TOKEN.type)) {
            variable_declaration();
            root = NULL;
        } else {
            switch (D_GLOBAL_TOKEN.type) {
            case T_PRINT:
                root = print_statement();
                break;
            case T_IDENTIFIER:
                root = assignment_statement();
                break;
            case T_IF:
                root = if_statement();
                match_semicolon = false;
                break;
            case T_WHILE:
                root = while_statement();
                match_semicolon = false;
                break;
            case T_FOR:
                root = for_statement();
                match_semicolon = false;
                break;
            case T_RIGHT_BRACE:
                match_token(T_RIGHT_BRACE);
                return_left = true;
                match_semicolon = false;
                break;
            default:
                syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Unexpected token %s",
                             tokenStrings[D_GLOBAL_TOKEN.type]);
                break;
            }
        }

        if (return_left) {
            return left;
        }

        if (match_semicolon) {
            match_token(T_SEMICOLON);
        }

        if (root) {
            if (left == NULL) {
                left = root;
            } else {
                left = create_ast_node(T_AST_GLUE, left, NULL, root, 0, NULL);
            }
        }
    }
}