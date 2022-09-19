/**
 * @file tree.h
 * @author Charles Averill
 * @brief Function headers for abstract syntax tree parsing
 * @date 09-Sep-2022
 */

#ifndef TREE_H
#define TREE_H

#include <stdlib.h>

#include "scan.h"
#include "utils/logging.h"

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
    /**Value of AST Node's Token*/
    union {
        /**Value of integer token*/
        int int_value;
        /**Index of identifier in global symbol table*/
        unsigned long int global_symbol_table_index;
    } value;
} ASTNode;

ASTNode* create_ast_node(TokenType ttype, ASTNode* left, ASTNode* mid, ASTNode* right,
                         unsigned long int value);
ASTNode* create_ast_leaf(TokenType ttype, unsigned long int value);
ASTNode* create_unary_ast_node(TokenType ttype, ASTNode* child, unsigned long int value);

#endif /* TREE_H */
