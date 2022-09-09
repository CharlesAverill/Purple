/**
 * @file expression.c
 * @author Charles Averill
 * @brief Logic for parsing into an AST
 * @date 09-Sep-2022
 */

#include "parse.h"

/**
 * Build a terminal AST Node for a given token, exit if not a valid primary token
 * @param  t The token to check and build
 * @return An AST Node built from the provided token, or an error if the token is non-terminal
 */
static ASTNode *create_terminal_node(Token t)
{
    ASTNode *out;

    switch (D_GLOBAL_TOKEN.type) {
    case T_INTEGER_LITERAL:
        out = create_ast_leaf(T_INTEGER_LITERAL, D_GLOBAL_TOKEN.value);
        scan(&D_GLOBAL_TOKEN);
        return out;
    default:
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Token: \"%s\"", token_strings[t.type]);
    }

    return NULL;
}

/**
 * Recursively parse binary expressions into an AST
 * @return ASTNode* An AST or AST Subtree of the binary expressions in D_INPUT_FILE
 */
ASTNode *parse_binary_expression(void)
{
    ASTNode *left;
    ASTNode *right;
    ASTNode *parent;
    TokenType ttype;

    // Get the integer literal on the left and scan the next token
    left = create_terminal_node(D_GLOBAL_TOKEN);

    if(D_GLOBAL_TOKEN.type == T_EOF) {
        return left;
    }

    ttype = D_GLOBAL_TOKEN.type;

    scan(&D_GLOBAL_TOKEN);

    right = parse_binary_expression();

    parent = create_ast_node(ttype, left, NULL, right, 0);

    return parent;
}
