/**
 * @file tree.c
 * @author Charles Averill
 * @brief Logic for constructing an Abstract Syntax Tree (AST)
 * @date 09-Sep-2022
 */

#include "tree.h"

/**
 * Constructs a new AST Node with the provided values
 * @param  ttype TokenType of the new AST Node
 * @param  left Left child subtree of the new AST Node
 * @param  mid Middle child subtree of the new AST Node
 * @param  right Right child subtree of the new AST Node
 * @param  value If TokenType == T_INTEGER_LITERAL, then the value of the integer literal
 * @return ASTNode* The pointer to a new AST Node with the provided values
 */
ASTNode* create_ast_node(TokenType ttype, ASTNode* left, ASTNode* mid, ASTNode* right, long value)
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
    //TODO fix this, it's a hack
    //Assigning to unions to change the other values of the union is undefined behavior
    out->value = value;

    return out;
}

/**
 * Constructs a new AST Leaf Node with the provided values
 * @param ttype TokenType of the new AST Node
 * @param value If TokenType == T_INTEGER_LITERAL, then the value of the integer literal
 * @return ASTNode* The pointer to a new AST Leaf Node with the provided values
 */
ASTNode* create_ast_leaf(TokenType ttype, long value)
{
    return create_ast_node(ttype, NULL, NULL, NULL, value);
}

/**
 * Constructs a new AST Unary Parent Node with the provided values
 * @param  ttype TokenType of the new AST Node
 * @param  child The AST Node's single child
 * @param  value If TokenType == T_INTEGER_LITERAL, then the value of the integer literal
 * @return ASTNode* The pointer to a new AST Unary Parent Node with the provided values
 */
ASTNode* create_unary_ast_node(TokenType ttype, ASTNode* child, int value)
{
    return create_ast_node(ttype, child, NULL, NULL, value);
}
