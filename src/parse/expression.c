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
 * @return An AST Node built from the provided Token, or an error if the Token is non-terminal
 */
static ASTNode* parse_terminal_node()
{
    ASTNode* out;
    SymbolTableEntry* entry;
    Token* t = &D_GLOBAL_TOKEN;

    if (TOKENTYPE_IS_LITERAL(t->type)) {
        out = create_ast_nonidentifier_leaf(
            t->type, TYPE_NUMBER_FROM_NUMBERTYPE_FROM_NUMBER(t->value.number_value));
    } else {
        switch (t->type) {
        case T_IDENTIFIER:
            if (!(entry = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, t->value.symbol_name))) {
                identifier_error(0, 0, 0, "Undeclared identifier %s", t->value.symbol_name);
            }

            if (entry->type.is_function) {
                return function_call_expression();
            } else {
                out = create_ast_identifier_leaf(T_IDENTIFIER, t->value.symbol_name);
            }
            break;
        case T_RIGHT_PAREN:
            return create_ast_nonidentifier_leaf(
                T_INTEGER_LITERAL, TYPE_NUMBER_FROM_NUMBERTYPE_FROM_NUMBER(NUMBER_INT(0)));
            break;
        default:
            syntax_error(0, 0, 0, "Unexpected end of expression, got \"%s\"",
                         tokenStrings[t->type]);
        }
    }

    scan();

    return out;
}

/**
 * @brief Look for prefix operators, otherwise pass through to parse_terminal_node
 * 
 * @return ASTNode* An AST Node containing data for a prefix operator, or a terminal AST Node
 */
ASTNode* prefix_operator_passthrough(void)
{
    ASTNode* out;

    purple_log(LOG_DEBUG, "Checking for prefix operators");

    if (D_GLOBAL_TOKEN.type == T_AMPERSAND) {
        purple_log(LOG_DEBUG, "Found address prefix operator");

        scan();
        out = prefix_operator_passthrough();

        if (out->ttype != T_IDENTIFIER) {
            syntax_error(0, 0, 0, "Cannot take the address of a non-identifier operand");
        }

        out->ttype = T_AMPERSAND;
        out->tree_type.pointer_depth++;

    } else if (D_GLOBAL_TOKEN.type == T_STAR) {
        purple_log(LOG_DEBUG, "Found dereference prefix operator");

        scan();
        out = prefix_operator_passthrough();

        if (out->ttype != T_IDENTIFIER && out->ttype != T_DEREFERENCE) {
            syntax_error(0, 0, 0, "Cannot dereference a non-pointer operand");
        }

        out->tree_type.pointer_depth--;
        // if (out->tree_type.pointer_depth < 0) {
        //     syntax_error(0, 0, 0, "Dereference operator on a non-pointer type is invalid");
        // }

        out = create_unary_ast_node(T_DEREFERENCE, out, TYPE_VOID, NULL);

        if (out->left) {
            out->value = out->left->value;
        }
    } else {
        purple_log(LOG_DEBUG, "Passing through to parse_terminal_node");
        out = parse_terminal_node();
    }

    return out;
}

/**
 * @brief Parse a function call expression into an AST
 * 
 * @return ASTNode* AST Node containing function call data
 */
ASTNode* function_call_expression(void)
{
    ASTNode* root;
    ASTNode* parameters;
    SymbolTableEntry* found_entry;

    purple_log(LOG_DEBUG, "Parsing function call statement");

    // Read identifier
    position ident_pos = D_GLOBAL_TOKEN.pos;
    match_token(T_IDENTIFIER);

    // Ensure identifier name has been declared
    purple_log(LOG_DEBUG, "Searching for function identifier name in global symbol table");
    if ((found_entry = STS_FIND(D_IDENTIFIER_BUFFER)) == NULL) {
        identifier_error(0, 0, 0, "Function dentifier name \"%s\" has not been declared",
                         D_IDENTIFIER_BUFFER);
    }

    match_token(T_LEFT_PAREN);
    parameters = parse_binary_expression();
    match_token(T_RIGHT_PAREN);

    // Make a terminal node for the identifier
    root = create_unary_ast_node(T_FUNCTION_CALL, parameters, found_entry->type,
                                 found_entry->symbol_name);
    root->tree_type.number_type =
        token_type_to_number_type(found_entry->type.value.function.return_type);
    add_position_info(root, ident_pos);

    return root;
}

/**
 * @brief Recursively parse binary expressions into an AST
 * 
 * @param previous_token_precedence The integer precedence value of the previous Token
 * @param nt_max Maximum NumberType encountered during AST generation
 * @return ASTNode*  An AST or AST Subtree of the binary expressions in D_INPUT_FILE
 */
static ASTNode* parse_binary_expression_recursive(int previous_token_precedence, NumberType* nt_max)
{
    ASTNode* left;
    ASTNode* right;
    TokenType current_ttype;

    // Get the terminal token (literal, variable identifier, etc) on the left and scan the next Token
    position pre_pos = D_GLOBAL_TOKEN.pos;
    left = prefix_operator_passthrough();
    *nt_max = MAX(*nt_max, left->tree_type.number_type);
    add_position_info(left, pre_pos);
    current_ttype = D_GLOBAL_TOKEN.type;
    if (current_ttype == T_SEMICOLON || current_ttype == T_RIGHT_PAREN) {
        left->is_rvalue = true;
        return left;
    }

    // While current Token has greater precedence than previous Token
    while (get_operator_precedence(D_GLOBAL_TOKEN) > previous_token_precedence ||
           (get_operator_precedence(D_GLOBAL_TOKEN) == previous_token_precedence &&
            rightAssociativeOperators[current_ttype])) {
        // Scan the next Token
        position pos = D_GLOBAL_TOKEN.pos;
        scan();

        // Recursively build the right AST subtree
        right = parse_binary_expression_recursive(operatorPrecedence[current_ttype], nt_max);

        if (current_ttype == T_ASSIGN) {
            // Left and right children must be swapped when assigning
            // to maintain right-associativity. We also want assignments to be
            // rvalues themselves.
            right->is_rvalue = true;
            ASTNode* temp = left;
            left = right;
            right = temp;
        } else {
            left->is_rvalue = right->is_rvalue = true;
        }

        // Join right subtree with current left subtree
        left = create_ast_node(current_ttype, left, NULL, right, TYPE_VOID, NULL);
        add_position_info(left, pos);

        // Update current_ttype and check for EOF
        current_ttype = D_GLOBAL_TOKEN.type;
        if (current_ttype == T_SEMICOLON || current_ttype == T_RIGHT_PAREN) {
            left->is_rvalue = true;
            return left;
        }
    }

    left->is_rvalue = true;
    return left;
}

/**
 * @brief Convenience wrapper for parse_binary_expression_recursive
 * 
 * @return ASTNode*  An AST or AST Subtree of the binary expressions in D_INPUT_FILE
 */
ASTNode* parse_binary_expression(void)
{
    purple_log(LOG_DEBUG, "Parsing binary expression");

    NumberType maximum = NT_INT1;
    ASTNode* out = parse_binary_expression_recursive(0, &maximum);
    out->largest_number_type = maximum;

    return out;
}
