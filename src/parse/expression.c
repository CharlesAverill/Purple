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
static int get_operator_precedence(Token t)
{
    int prec = operatorPrecedence[t.type];

    if (prec == 0) {
        syntax_error(0, 0, 0, "Expected operator but got %s", tokenStrings[t.type]);
    }

    return prec;
}

/**
 * @brief Build a terminal AST Node for a given Token, exit if not a valid primary Token
 * 
 * @param t The Token to check and build
 * @return An AST Node built from the provided Token, or an error if the Token is non-terminal
 */
static ASTNode* create_terminal_node(Token* t)
{
    ASTNode* out;
    SymbolTableEntry* entry;

    if (TOKENTYPE_IS_LITERAL(t->type)) {
        out = create_ast_nonidentifier_leaf(t->type, TYPE_NUMBER_VAL(t->value.number_value));
    } else {
        switch (t->type) {
        case T_IDENTIFIER:
            if (!find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, t->value.symbol_name)) {
                identifier_error(0, 0, 0, "Undeclared identifier %s", t->value.symbol_name);
            }
            out = create_ast_identifier_leaf(T_IDENTIFIER, t->value.symbol_name);
            break;
        default:
            syntax_error(0, 0, 0, "Tried creating a terminal node with token type \"%s\"",
                         tokenStrings[t->type]);
        }
    }

    scan(t);

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
    position pre_pos = D_GLOBAL_TOKEN.pos;
    left = create_terminal_node(&D_GLOBAL_TOKEN);
    printf("Terminal\n");
    add_position_info(left, pre_pos);
    current_ttype = D_GLOBAL_TOKEN.type;
    if (current_ttype == T_SEMICOLON || current_ttype == T_RIGHT_PAREN) {
        return left;
    }

    // While current Token has greater precedence than previous Token
    while (get_operator_precedence(D_GLOBAL_TOKEN) > previous_token_precedence) {
        // Scan the next Token
        position pos = D_GLOBAL_TOKEN.pos;
        scan(&D_GLOBAL_TOKEN);

        // Recursively build the right AST subtree
        right = parse_binary_expression(operatorPrecedence[current_ttype]);

        // Join right subtree with current left subtree
        left = create_ast_node(current_ttype, left, NULL, right, TYPE_VOID, NULL);
        printf("Add to left\n");
        add_position_info(left, pos);

        // Update current_ttype and check for EOF
        current_ttype = D_GLOBAL_TOKEN.type;
        if (current_ttype == T_SEMICOLON || current_ttype == T_RIGHT_PAREN) {
            return left;
        }
    }

    return left;
}
