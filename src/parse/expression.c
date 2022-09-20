/**
 * @file expression.c
 * @author Charles Averill
 * @brief Logic for parsing expressions into an AST
 * @date 09-Sep-2022
 */

#include "data.h"
#include "parse.h"

/**
 * @brief Get the integer operator precedence value of a Token
 * 
 * @param t Token to check preference of
 * @return int Precedence of Token's type
 */
static int get_operatorPrecedence(Token t)
{
    int prec = operatorPrecedence[t.type];

    if (prec == 0) {
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Expected operator but got %s \"%d\"",
                     tokenStrings[t.type], t.value);
    }

    return prec;
}

/**
 * @brief Build a terminal AST Node for a given Token, exit if not a valid primary Token
 * 
 * @param t The Token to check and build
 * @return An AST Node built from the provided Token, or an error if the Token is non-terminal
 */
static ASTNode* create_terminal_node(Token t)
{
    ASTNode* out;
    SymbolTableEntry* entry;

    switch (D_GLOBAL_TOKEN.type) {
    case T_INTEGER_LITERAL:
        out = create_ast_nonidentifier_leaf(T_INTEGER_LITERAL, D_GLOBAL_TOKEN.value.int_value);
        break;
    case T_IDENTIFIER:
        out = create_ast_identifier_leaf(T_IDENTIFIER, t.value.symbol_name);
        break;
    default:
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Token: \"%s\"", tokenStrings[t.type]);
    }

    scan(&D_GLOBAL_TOKEN);

    return out;
}

/**
 * @brief Recursively parse binary expressions into an AST
 * 
 * @param previous_token_precedence The integer precedence value of the previous Token
 * @return ASTNode*  An AST or AST Subtree of the binary expressions in D_INPUT_FILE
 */
ASTNode* parse_binary_expression(int previous_token_precedence)
{
    ASTNode* left;
    ASTNode* right;
    TokenType current_ttype;

    // Get the intlit on the left and scan the next Token
    left = create_terminal_node(D_GLOBAL_TOKEN);
    current_ttype = D_GLOBAL_TOKEN.type;
    if (current_ttype == T_SEMICOLON) {
        return left;
    }

    // While current Token has greater precedence than previous Token
    while (get_operatorPrecedence(D_GLOBAL_TOKEN) > previous_token_precedence) {
        // Scan the next Token
        scan(&D_GLOBAL_TOKEN);

        // Recursively build the right AST subtree
        right = parse_binary_expression(operatorPrecedence[current_ttype]);

        // Join right subtree with current left subtree
        left = create_ast_node(current_ttype, left, NULL, right, 0, NULL);

        // Update current_ttype and check for EOF
        current_ttype = D_GLOBAL_TOKEN.type;
        if (current_ttype == T_SEMICOLON) {
            return left;
        }
    }

    return left;
}
