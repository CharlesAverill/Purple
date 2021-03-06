/**
 * @file
 * @author CharlesAverill
 * @date   11-Oct-2021
 * @brief Function for parsing statements and compiling them into ASM
*/

#include "parse.h"
#include "symbol_table.h"

// Extern variables
FILE *ASM_OUTPUT;
ASM_Generators generators;

/**
 * Parse print statement
 */
static AST_Node *print_statement(void)
{
    AST_Node *root;

    // First token must be print
    match(T_PRINT);

    // Parse expression into AST
    root = parse_binary_expression(0);
    root = make_unary_ast_node(T_PRINT, root, 0);

    return root;
}

/**
 * Parse variable assignment statement
 */
static AST_Node *assignment_statement(void)
{
    AST_Node *left;
    AST_Node *right;
    AST_Node *root;

    // First token must be an existing identifier
    match(T_IDENTIFIER);

    // Check position of variable in Global symbol table
    int position = symbol_exists(D_GLOBAL_SYMBOL_TABLE, D_IDENTIFIER_BUFFER);
    if (position == -1) {
        print_symbol_table(D_GLOBAL_SYMBOL_TABLE);
        fprintf(stderr, "Undefined variable %s on line %d\n", D_IDENTIFIER_BUFFER, D_LINE_NUMBER);
        shutdown(1);
    }

    int len = strlen(D_IDENTIFIER_BUFFER);
    char *identifier = malloc(len);
    strncpy(identifier, D_IDENTIFIER_BUFFER, len);
    // Build AST leaf for left value identifier
    right = make_ast_leaf(T_AST_LEFT_VALUE_IDENTIFIER, (long)identifier);

    // Match for an equals token
    match(T_ASSIGNMENT);

    // Build AST for assignment expression
    left = parse_binary_expression(0);

    // Assembly left and right into AST
    root = make_ast_node(T_ASSIGNMENT, left, NULL, right, 0);

    return root;
}

/**
 * Parse an if statement
 */
AST_Node *if_statement(symbol_table *parent_table)
{
    AST_Node *condition_root = NULL;
    AST_Node *true_root = NULL;
    AST_Node *false_root = NULL;

    match(T_IF);
    match(T_LEFT_PARENTHESIS);

    condition_root = parse_binary_expression(0);

    if (condition_root->ttype < T_EQUALS || condition_root->ttype > T_GREATER_EQUAL) {
        fprintf(stderr, "Uncrecognized comparison on line %d\n", D_LINE_NUMBER);
        shutdown(1);
    }

    match(T_RIGHT_PARENTHESIS);

    true_root = parse_compound_statement(parent_table);

    if (GToken._token == T_ELSE) {
        scan(&GToken);
        false_root = parse_compound_statement(parent_table);
    }

    return make_ast_node(T_IF, condition_root, true_root, false_root, 0);
}

/**
 * Parse an while statement
 */
AST_Node *while_statement(symbol_table *parent_table)
{
    AST_Node *condition_root = NULL;
    AST_Node *body_root = NULL;

    match(T_WHILE);
    match(T_LEFT_PARENTHESIS);

    condition_root = parse_binary_expression(0);

    if (condition_root->ttype < T_EQUALS || condition_root->ttype > T_GREATER_EQUAL) {
        fprintf(stderr, "Uncrecognized comparison on line %d\n", D_LINE_NUMBER);
        shutdown(1);
    }

    match(T_RIGHT_PARENTHESIS);

    body_root = parse_compound_statement(parent_table);

    return make_ast_node(T_WHILE, condition_root, NULL, body_root, 0);
}

/**
 * Parse with-as statement
 */
static AST_Node *with_as_statement(symbol_table *parent_table)
{
    //TODO: Re implement this once we have locals
    /*
    AST_Node *expression_root;
    AST_Node *declaration_root;
    AST_Node *assignment_root;
    AST_Node *body_root;

    match(T_WITH);
    match(T_LEFT_PARENTHESIS);

    expression_root = parse_binary_expression(0);

    match(T_AS);

    // Must provide type
    Token_Type datatype = match_datatype();
    match(T_IDENTIFIER);

    // Add to symbol table
    int position = insert_global_symbol(D_IDENTIFIER_BUFFER, datatype);

    // Copy buffer for removal later
    char as_identifier[D_MAX_IDENTIFIER_LENGTH + 1];
    strcpy(as_identifier, D_IDENTIFIER_BUFFER);

    match(T_RIGHT_PARENTHESIS);

    // Build AST leaf for left value identifier
    declaration_root = make_ast_leaf(T_AST_LEFT_VALUE_IDENTIFIER, position);

    // Assembly left and right into AST
    assignment_root = make_ast_node(T_ASSIGNMENT, expression_root, NULL, declaration_root, 0);

    body_root = parse_compound_statement();

    remove_global_symbol(as_identifier);

	// Essentially we are saying:
	// <type> temp_var = <expression>
	// do compound statement
	// Dereference temp_var
    return make_ast_node(T_AST_GLUE, assignment_root, NULL, body_root, 0);
    */
}

/**
 * Parse for statement
 */
static AST_Node *for_statement(symbol_table *parent_table)
{
    AST_Node *assignment_root;
    AST_Node *condition_root;
    AST_Node *post_operation_root;
    AST_Node *body_root;
    AST_Node *output;

    match(T_FOR);
    match(T_LEFT_PARENTHESIS);

    // Get the loop assignment statement
    assignment_root = parse_statement(parent_table);

    match(T_SEMICOLON);

    // Get the loop condition
    condition_root = parse_binary_expression(0);
    if (condition_root->ttype < T_EQUALS || condition_root->ttype > T_GREATER_EQUAL) {
        fprintf(stderr, "Uncrecognized comparison on line %d\n", D_LINE_NUMBER);
        shutdown(1);
    }

    match(T_SEMICOLON);

    // Get the post-loop operation
    post_operation_root = parse_statement(parent_table);

    // Close the statement
    match(T_RIGHT_PARENTHESIS);

    // Fill in the body
    body_root = parse_compound_statement(parent_table);

    // Glue the subtrees together
    output = make_ast_node(T_AST_GLUE, body_root, NULL, post_operation_root, 0);
    output = make_ast_node(T_WHILE, condition_root, NULL, output, 0);
    return make_ast_node(T_AST_GLUE, assignment_root, NULL, output, 0);
}

AST_Node *parse_statement(symbol_table *parent_table)
{
    AST_Node *root;

    switch (GToken._token) {
    case T_PRINT:
        root = print_statement();
        break;
    case T_INT:
        variable_declaration();
        root = NULL;
        break;
    case T_IDENTIFIER:
        root = assignment_statement();
        break;
    case T_IF:
        root = if_statement(parent_table);
        break;
    case T_FOR:
        root = for_statement(parent_table);
        break;
    case T_WHILE:
        root = while_statement(parent_table);
        break;
    case T_WITH:
        root = with_as_statement(parent_table);
        break;
    default:
        fprintf(stderr, "Syntax error on line %d, token \"%s\"\n", D_LINE_NUMBER,
                token_strings[GToken._token]);
        shutdown(1);
    }

    return root;
}

/**
 * Parse a compound statement (anything between braces) and return its AST
 */
AST_Node *parse_compound_statement(symbol_table *parent_table)
{
    AST_Node *left = NULL;
    AST_Node *root;

    // Initialize the symbol table for this local scope
    symbol_table *scope_symbol_table;
    if (parent_table != NULL) {
        scope_symbol_table = make_symbol_table(parent_table);
    } else {
        scope_symbol_table = D_GLOBAL_SYMBOL_TABLE;
    }

    match(T_LEFT_BRACE);

    while (1) {
        root = parse_statement(scope_symbol_table);

        if (root) {
            // Only some statements require a semicolon
            if (root->ttype == T_PRINT || root->ttype == T_ASSIGNMENT) {
                match(T_SEMICOLON);
            }

            if (left == NULL) {
                left = root;
            } else {
                left = make_ast_node(T_AST_GLUE, left, NULL, root, 0);
            }
        }

        if (GToken._token == T_RIGHT_BRACE) {
            match(T_RIGHT_BRACE);
            return make_ast_node(T_SCOPE, left, NULL, NULL, scope_symbol_table);
        }
    }
}
