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
        scan();
    } else {
        syntax_error(0, 0, 0, "Expected token \"%s\"", tokenStrings[type]);
    }
}

TokenType match_tokens(TokenType types[], int n_types)
{
    char possible_types[256];

    int last_type_length = 0;
    for (int i = 0; i < n_types; i++) {
        sprintf(possible_types + (sizeof(char) * last_type_length), "%s ", tokenStrings[types[i]]);
        last_type_length += strlen(tokenStrings[types[i]]) + 1;

        if (D_GLOBAL_TOKEN.type == types[i]) {
            scan();
            return types[i];
        }
    }

    syntax_error(0, 0, 0, "Expected one of: [ %s] but got \'%s\'", possible_types,
                 tokenStrings[D_GLOBAL_TOKEN.type]);
    return 0;
}

int match_type(Number* out)
{
    D_SCANNING_TYPE = true;

    TokenType ttype =
        match_tokens((TokenType[]){T_VOID, T_BOOL, T_BYTE, T_CHAR, T_SHORT, T_INT, T_LONG}, 7);

    int pointer_depth;
    for (pointer_depth = 0; D_GLOBAL_TOKEN.type == T_STAR; pointer_depth++) {
        scan();
    }

    if (ttype == T_VOID && pointer_depth == 0) {
        return 1;
    }

    *out = (Number){
        .type = token_type_to_number_type(ttype), .pointer_depth = pointer_depth, .value = 0};

    D_SCANNING_TYPE = false;
    return 0;
}

/**
 * @brief Ensure current token is a type token, and scan the next token if so
 * 
 * @return NumberType Type of variable
 */
TokenType check_for_type(void)
{
    TokenType ttype = D_GLOBAL_TOKEN.type;
    if (TOKENTYPE_IS_TYPE(ttype)) {
        scan();
    } else {
        syntax_error(0, 0, 0, "Expected type but got \"%s\"", tokenStrings[ttype]);
    }

    return ttype;
}

/**
 * @brief Parse a print statement into an AST
 * 
 * @return ASTNode* AST for print statement
 */
static ASTNode* print_statement(void)
{
    ASTNode* root;

    purple_log(LOG_DEBUG, "Parsing print statement");

    match_token(T_PRINT);
    position print_position = D_GLOBAL_TOKEN.pos;

    // Parse printed value
    root = parse_binary_expression();

    root = create_unary_ast_node(T_PRINT, root, TYPE_VOID, NULL);
    add_position_info(root, print_position);

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
    position ident_pos = D_GLOBAL_TOKEN.pos;
    match_token(T_IDENTIFIER);

    // Ensure identifier name has been declared
    purple_log(LOG_DEBUG, "Searching for identifier name in global symbol table");
    if ((found_entry = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, D_IDENTIFIER_BUFFER)) ==
        NULL) {
        identifier_error(0, 0, 0, "Identifier name \"%s\" has not been declared",
                         D_IDENTIFIER_BUFFER);
    }

    // Make a terminal node for the identifier
    right = create_ast_identifier_leaf(T_IDENTIFIER, found_entry->symbol_name);
    add_position_info(right, ident_pos);

    match_token(T_ASSIGN);
    position assign_pos = D_GLOBAL_TOKEN.pos;

    // Parse assignment expression
    left = parse_binary_expression();

    right->is_char_arithmetic = left->is_char_arithmetic;

    // Create subtree for assignment statement
    root = create_ast_node(T_ASSIGN, left, NULL, right, TYPE_VOID, NULL);
    add_position_info(root, assign_pos);

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

    condition = parse_binary_expression();
    position condition_pos = D_GLOBAL_TOKEN.pos;

    if (!TOKENTYPE_IS_COMPARATOR(condition->ttype) &&
        !TOKENTYPE_IS_LOGICAL_OPERATOR(condition->ttype)) {
        syntax_error(0, 0, 0, "Condition clauses must use a logical or comparison operator");
    }

    match_token(T_RIGHT_PAREN);

    true_branch = parse_statements();

    if (D_GLOBAL_TOKEN.type == T_ELSE) {
        match_token(T_ELSE);
        false_branch = parse_statements();
    }

    condition = create_ast_node(T_IF, condition, true_branch, false_branch, TYPE_VOID, NULL);
    add_position_info(condition, condition_pos);

    return condition;
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

    condition = parse_binary_expression();
    position condition_pos = D_GLOBAL_TOKEN.pos;

    if (!TOKENTYPE_IS_COMPARATOR(condition->ttype) &&
        !TOKENTYPE_IS_LOGICAL_OPERATOR(condition->ttype)) {
        syntax_error(0, 0, 0, "Condition clauses must use a logical or comparison operator");
    }

    match_token(T_RIGHT_PAREN);

    body = parse_statements();

    if (D_GLOBAL_TOKEN.type == T_ELSE) {
        purple_log(LOG_DEBUG, "Encountered while-else statement");

        match_token(T_ELSE);
        else_body = parse_statements();
    }

    condition = create_ast_node(T_WHILE, condition, body, else_body, TYPE_VOID, NULL);
    add_position_info(condition, condition_pos);

    return condition;
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
    position for_position = D_GLOBAL_TOKEN.pos;

    match_token(T_SEMICOLON);

    condition = parse_binary_expression();
    if (!TOKENTYPE_IS_COMPARATOR(condition->ttype) &&
        !TOKENTYPE_IS_LOGICAL_OPERATOR(condition->ttype)) {
        syntax_error(0, 0, 0, "Condition clauses must use a logical or comparison operator");
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

    out = create_ast_node(T_AST_GLUE, for_postamble, NULL, else_body, TYPE_VOID, NULL);
    out = create_ast_node(T_WHILE, condition, body, out, TYPE_VOID, NULL);
    add_position_info(out, for_position);
    out = create_ast_node(T_AST_GLUE, for_preamble, NULL, out, TYPE_VOID, NULL);
    return out;
}

static ASTNode* return_statement(void)
{
    ASTNode* out;

    SymbolTableEntry* entry =
        find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, D_CURRENT_FUNCTION_BUFFER);
    if (!entry) {
        fatal(RC_COMPILER_ERROR,
              "return_statement received symbol name \"%s\", which is not an identifier",
              D_CURRENT_FUNCTION_BUFFER);
    } else if (!entry->type.is_function) {
        fatal(RC_COMPILER_ERROR,
              "return_statement received an identifier name that is not a function: \"%s\"",
              D_CURRENT_FUNCTION_BUFFER);
    }

    match_token(T_RETURN);

    if (entry->type.value.function.return_type == T_VOID) {
        return create_unary_ast_node(T_RETURN, NULL, TYPE_VOID, D_CURRENT_FUNCTION_BUFFER);
    }

    out = parse_binary_expression();

    return create_unary_ast_node(T_RETURN, out, entry->type, D_CURRENT_FUNCTION_BUFFER);
}

/**
 * @brief Parse a set of statements into ASTs and generate them into an AST
 * 
 * @return AST for a group of statements
 */
ASTNode* parse_statements(void)
{
    purple_log(LOG_DEBUG, "Parsing statements");

    ASTNode* left = NULL;
    ASTNode* root;

    SymbolTableEntry* symbol;

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
            // case T_IDENTIFIER:
            //     symbol = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE,
            //                                      D_GLOBAL_TOKEN.value.symbol_name);
            //     if (!symbol) {
            //         identifier_error(0, 0, 0, "Use of undeclared identifier \'%s\'",
            //                          D_GLOBAL_TOKEN.value.symbol_name);
            //     } else if (symbol->type.is_function) {
            //         root = function_call_expression();
            //     } else {
            //         root = assignment_statement();
            //     }
            //     break;
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
            case T_RETURN:
                root = return_statement();
                break;
            default:
                // syntax_error(0, 0, 0, "Unexpected token %s", tokenStrings[D_GLOBAL_TOKEN.type]);
                root = parse_binary_expression();
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
                left = create_ast_node(T_AST_GLUE, left, NULL, root, TYPE_VOID, NULL);
            }
        }
    }
}
