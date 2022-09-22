/**
 * @file tree.h
 * @author Charles Averill
 * @brief Function headers for abstract syntax tree parsing
 * @date 09-Sep-2022
 */

#ifndef TREE
#define TREE

#include <stdlib.h>

#include "scan.h"
#include "translate/symbol_table.h"
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
    /**Type of number token*/
    NumberType number_type;
    /**Value of AST Node's Token*/
    union {
        /**Value of integer token*/
        int int_value;
        /**Name of this identifier token*/
        char symbol_name[D_MAX_IDENTIFIER_LENGTH];
    } value;
} ASTNode;

ASTNode* create_ast_node(TokenType ttype, ASTNode* left, ASTNode* mid, ASTNode* right,
                         unsigned long int value, char* symbol_name);
ASTNode* create_ast_nonidentifier_leaf(TokenType ttype, unsigned long int value);
ASTNode* create_ast_identifier_leaf(TokenType ttype, char* symbol_name);
ASTNode* create_unary_ast_node(TokenType ttype, ASTNode* child, unsigned long int value);
void ast_debug_level_order(ASTNode* root, LogLevel log_level);

#endif /* TREE */
