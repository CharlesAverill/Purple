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
 * @param value If Token is a literal, then the value of the literal
 * @param symbol_name if TokenType == T_IDENTIFIER, then the string for the identifier
 * @return ASTNode* The pointer to a new AST Node with the provided values
 */
ASTNode* create_ast_node(TokenType ttype, ASTNode* left, ASTNode* mid, ASTNode* right, Number value,
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
    if (TOKENTYPE_IS_LITERAL(ttype)) {
        out->value.number_value = value.value;
        out->number_type = value.type;
    } else if (TOKENTYPE_IS_IDENTIFIER(ttype)) {
        strcpy(out->value.symbol_name, symbol_name);

        SymbolTableEntry* found_entry = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, symbol_name);
        if (found_entry == NULL) {
            fatal(RC_COMPILER_ERROR,
                  "create_ast_node received identifier name that is not defined in the GST");
        }

        out->number_type = found_entry->number_type;
    } else if (TOKENTYPE_IS_BINARY_ARITHMETIC(ttype)) {
        out->number_type = left->number_type;
    } else if (TOKENTYPE_IS_COMPARATOR(ttype)) {
        out->number_type = NT_INT1;
    }

    return out;
}

/**
 * @brief Constructs a new AST Leaf Node with the provided values for a token that is not an identifier
 * 
 * @param ttype TokenType of the new AST Node
 * @param value If Token is a literal, then the value of the literal
 * @return ASTNode* The pointer to a new AST Leaf Node with the provided values
 */
ASTNode* create_ast_nonidentifier_leaf(TokenType ttype, Number value)
{
    return create_ast_node(ttype, NULL, NULL, NULL, value, NULL);
}

/**
 * @brief Constructs a new AST Leaf Node with the provided values for a token that is an identifier
 * 
 * @param ttype TokenType of the new AST Node
 * @param gst_entry SymbolTableEntry in Global Symbol Table for this token
 * @return ASTNode* The pointer to a new AST Leaf Node with the provided values
 */
ASTNode* create_ast_identifier_leaf(TokenType ttype, char* symbol_name)
{
    return create_ast_node(ttype, NULL, NULL, NULL, NUMBER_INT(0), symbol_name);
}

/**
 * @brief Constructs a new AST Unary Parent Node with the provided values
 * 
 * @param ttype TokenType of the new AST Node
 * @param child The AST Node's single child
 * @param value If Token is a literal, then the value of the literal
 * @return ASTNode* The pointer to a new AST Unary Parent Node with the provided values
 */
ASTNode* create_unary_ast_node(TokenType ttype, ASTNode* child, Number value)
{
    return create_ast_node(ttype, child, NULL, NULL, value, NULL);
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
        if (TOKENTYPE_IS_IDENTIFIER(root->ttype)) {
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
