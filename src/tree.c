/**
 * @file tree.c
 * @author Charles Averill
 * @brief Logic for constructing an Abstract Syntax Tree (AST)
 * @date 09-Sep-2022
 */

#include <stdio.h>
#include <string.h>

#include "data.h"

#include "tree.h"

/**
 * @brief Constructs a new AST Node with the provided values
 * 
 * @param ttype TokenType of the new AST Node
 * @param left Left child subtree of the new AST Node
 * @param mid Middle child subtree of the new AST Node
 * @param right Right child subtree of the new AST Node
 * @param type Type of the new AST Node
 * @param symbol_name The identifier for the provided Token information
 * @return ASTNode* The pointer to a new AST Node with the provided values
 */
ASTNode* create_ast_node(TokenType ttype, ASTNode* left, ASTNode* mid, ASTNode* right, Type type,
                         char* symbol_name)
{
    ASTNode* out;

    // Allocate memory for new node
    out = (ASTNode*)malloc(sizeof(ASTNode));
    if (out == NULL) {
        fatal(RC_MEMORY_ERROR, "Unable to allocate memory for new AST Node");
    }

    // Assign values
    out->ttype = ttype;
    out->left = left;
    out->mid = mid;
    out->right = right;
    if (TOKENTYPE_IS_LITERAL(ttype) && TOKENTYPE_IS_NUMBER_TYPE(type.token_type)) {
        out->value.number_value = type.value.number.value;
        out->tree_type.number_type = type.value.number.number_type;
        out->is_char_arithmetic = ttype == T_CHAR || ttype == T_CHAR_LITERAL;
    } else if (ttype == T_IDENTIFIER || ttype == T_FUNCTION_CALL) {
        if (symbol_name == NULL) {
            fatal(RC_COMPILER_ERROR,
                  "Tried to create identifier node, but passed symbol_name is NULL");
        }

        strcpy(out->value.symbol_name, symbol_name);

        SymbolTableEntry* found_entry = STS_FIND(symbol_name);
        if (found_entry == NULL) {
            fatal(RC_COMPILER_ERROR,
                  "create_ast_node received identifier name that is not defined in the GST");
        }

        out->tree_type.number_type = found_entry->type.value.number.number_type;
        out->is_char_arithmetic = found_entry->type.token_type == T_CHAR ||
                                  found_entry->type.token_type == T_CHAR_LITERAL;
    } else if (TOKENTYPE_IS_BINARY_ARITHMETIC(ttype)) {
        out->tree_type.number_type = left->tree_type.number_type;
        out->is_char_arithmetic = left->is_char_arithmetic;
    } else if (TOKENTYPE_IS_COMPARATOR(ttype)) {
        out->tree_type.number_type = NT_INT1;
    } else if (ttype == T_FUNCTION_DECLARATION) {
        if (symbol_name == NULL) {
            fatal(RC_COMPILER_ERROR,
                  "Tried to create function declaration node, but passed symbol_name is NULL");
        }

        strcpy(out->value.symbol_name, symbol_name);

        SymbolTableEntry* found_entry = STS_FIND(symbol_name);
        if (found_entry == NULL) {
            fatal(RC_COMPILER_ERROR,
                  "create_ast_node received function name that is not defined in the GST");
        }
    } else if (ttype == T_RETURN) {
        strcpy(out->value.symbol_name, symbol_name);
    }

    return out;
}

/**
 * @brief Add position information to an ASTNode
 * 
 * @param dest Destination ASTNode pointer
 * @param p Position information
 */
void add_position_info(ASTNode* dest, position p)
{
    strcpy(dest->filename, p.filename);
    dest->line_number = p.line_number;
    dest->char_number = p.char_number;
}

/**
 * @brief Constructs a new AST Leaf Node with the provided values for a token that is not an identifier
 * 
 * @param ttype TokenType of the new AST Node
 * @param type Type of the new AST Node
 * @return ASTNode* The pointer to a new AST Leaf Node with the provided values
 */
ASTNode* create_ast_nonidentifier_leaf(TokenType ttype, Type type)
{
    return create_ast_node(ttype, NULL, NULL, NULL, type, NULL);
}

/**
 * @brief Constructs a new AST Leaf Node with the provided values for a token that is an identifier
 * 
 * @param ttype TokenType of the new AST Node
 * @param symbol_name The identifier for the provided Token information
 * @return ASTNode* The pointer to a new AST Leaf Node with the provided values
 */
ASTNode* create_ast_identifier_leaf(TokenType ttype, char* symbol_name)
{
    return create_ast_node(ttype, NULL, NULL, NULL, TYPE_VOID, symbol_name);
}

/**
 * @brief Constructs a new AST Unary Parent Node with the provided values
 * 
 * @param ttype TokenType of the new AST Node
 * @param child The AST Node's single child
 * @param type Type of the new AST Node
 * @param symbol_name The identifier for the provided Token information
 * @return ASTNode* The pointer to a new AST Unary Parent Node with the provided values
 */
ASTNode* create_unary_ast_node(TokenType ttype, ASTNode* child, Type type, char* symbol_name)
{
    return create_ast_node(ttype, child, NULL, NULL, type, symbol_name);
}

/**
 * @brief Get the height of an AST Node
 * 
 * @param node Node to get height of
 * @return int Height of node
 */
static int ast_node_height(ASTNode* node)
{
    if (node == NULL) {
        return 0;
    }

    int left_height = ast_node_height(node->left);
    int right_height = ast_node_height(node->right);
    if (left_height > right_height) {
        return left_height + 1;
    }

    return right_height + 1;
}

/**
 * @brief Print out a single level of an AST in order from left to right
 * 
 * @param root Root of AST
 * @param height Level of tree to be printed out
 * @param log_level Log level to print at
 */
static void ast_debug_current_level(ASTNode* root, int height, LogLevel log_level)
{
    if (root == NULL) {
        return;
    } else if (height == 1) {
        if (root->ttype == T_IDENTIFIER) {
            purple_log(log_level, "%s:%s", tokenStrings[root->ttype], root->value.symbol_name);
        } else if (TOKENTYPE_IS_LITERAL(root->ttype)) {
            purple_log(log_level, "%s:%d", tokenStrings[root->ttype], root->value.number_value);
        } else {
            purple_log(log_level, "%s", tokenStrings[root->ttype]);
        }
    } else if (height > 1) {
        ast_debug_current_level(root->left, height - 1, log_level);
        ast_debug_current_level(root->right, height - 1, log_level);
    }
}

/**
 * @brief Print out an AST's level order traversal
 * 
 * @param root Root of AST
 * @param log_level Log level to print at
 */
void ast_debug_level_order(ASTNode* root, LogLevel log_level)
{
    purple_log(log_level, "---Level Order AST Traversal---");
    int height = ast_node_height(root);
    for (int i = 1; i <= height; i++) {
        purple_log(log_level, "[LEVEL %d]", i);
        ast_debug_current_level(root, i, log_level);
    }
}
