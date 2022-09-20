/**
 * @file tree.c
 * @author Charles Averill
 * @brief Logic for constructing an Abstract Syntax Tree (AST)
 * @date 09-Sep-2022
 */

#include <stdio.h>
#include <string.h>

#include "tree.h"

/**
 * @brief Constructs a new AST Node with the provided values
 * 
 * @param ttype TokenType of the new AST Node
 * @param left Left child subtree of the new AST Node
 * @param mid Middle child subtree of the new AST Node
 * @param right Right child subtree of the new AST Node
 * @param value If TokenType == T_INTEGER_LITERAL, then the value of the integer literal
 * @param symbol_name if TokenType == T_IDENTIFIER, then the string for the identifier
 * @return ASTNode* The pointer to a new AST Node with the provided values
 */
ASTNode* create_ast_node(TokenType ttype, ASTNode* left, ASTNode* mid, ASTNode* right,
                         unsigned long int value, char* symbol_name)
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
    if (ttype == T_INTEGER_LITERAL) {
        out->value.int_value = value;
    } else if (TOKENTYPE_IS_IDENTIFIER(ttype)) {
        strcpy(out->value.symbol_name, symbol_name);
    }

    return out;
}

/**
 * @brief Constructs a new AST Leaf Node with the provided values for a token that is not an identifier
 * 
 * @param ttype TokenType of the new AST Node
 * @param value If TokenType == T_INTEGER_LITERAL, then the value of the integer literal
 * @return ASTNode* The pointer to a new AST Leaf Node with the provided values
 */
ASTNode* create_ast_nonidentifier_leaf(TokenType ttype, unsigned long int value)
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
    return create_ast_node(ttype, NULL, NULL, NULL, 0, symbol_name);
}

/**
 * @brief Constructs a new AST Unary Parent Node with the provided values
 * 
 * @param ttype TokenType of the new AST Node
 * @param child The AST Node's single child
 * @param value If TokenType == T_INTEGER_LITERAL, then the value of the integer literal
 * @return ASTNode* The pointer to a new AST Unary Parent Node with the provided values
 */
ASTNode* create_unary_ast_node(TokenType ttype, ASTNode* child, unsigned long int value)
{
    return create_ast_node(ttype, child, NULL, NULL, value, NULL);
}

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

static void ast_debug_current_level(ASTNode* root, int height, LogLevel log_level)
{
    if (root == NULL) {
        return;
    } else if (height == 1) {
        if (TOKENTYPE_IS_IDENTIFIER(root->ttype)) {
            purple_log(log_level, "%s:%s", tokenStrings[root->ttype], root->value.symbol_name);
        } else if (TOKENTYPE_IS_LITERAL(root->ttype)) {
            purple_log(log_level, "%s:%d", tokenStrings[root->ttype], root->value.int_value);
        } else {
            purple_log(log_level, "%s", tokenStrings[root->ttype]);
        }
    } else if (height > 1) {
        ast_debug_current_level(root->left, height - 1, log_level);
        ast_debug_current_level(root->right, height - 1, log_level);
    }
}

void ast_debug_level_order(ASTNode* root, LogLevel log_level)
{
    purple_log(log_level, "---Level Order AST Traversal---");
    int height = ast_node_height(root);
    for (int i = 1; i <= height; i++) {
        purple_log(log_level, "[LEVEL %d]", i);
        ast_debug_current_level(root, i, log_level);
    }
}
