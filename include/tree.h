/**
 * @file tree.h
 * @author Charles Averill
 * @brief Function headers for abstract syntax tree parsing
 * @date 09-Sep-2022
 */

#ifndef TREE_H
#define TREE_H

#include <stdlib.h>

#include "errors_warnings.h"
#include "scan.h"

/**
 * @brief Component of the abstract syntax tree built during parsing
 */
typedef struct ASTNode {
    /**@brief The TokenType of the given token*/
    TokenType ttype;
    /**The left child of the AST Node*/
    struct ASTNode* left;
    /**The middle child of the AST Node*/
    struct ASTNode* mid;
    /**The right child of the AST Node*/
    struct ASTNode* right;
    /**Union containing either the value of an integer literal, or the identifier of a symbol*/
    int value;
} ASTNode;

ASTNode* create_ast_node(TokenType ttype, ASTNode* left, ASTNode* mid, ASTNode* right, long value);
ASTNode* create_ast_leaf(TokenType ttype, long value);
ASTNode* create_unary_ast_node(TokenType ttype, ASTNode* child, int value);

#endif /* TREE_H */
