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
    /**Numerical type of tree*/
    Number tree_type;
    /**Largest NumberType in subtree*/
    NumberType largest_number_type;
    /**Whether or not this ASTNode contains an RValue*/
    bool is_rvalue;
    /**Filename of this Token*/
    char filename[256];
    /**Line number of this Token*/
    int line_number;
    /**Character number of this Token*/
    int char_number;
    /**Size of function_call_arguments*/
    unsigned long long int num_args;
    /**Array of arguments for a function call node*/
    struct ASTNode** function_call_arguments;
    /**Value of AST Node's Token*/
    union {
        /**Value of integer token*/
        number_literal_type number_value;
        /**Name of this identifier token*/
        char symbol_name[MAX_IDENTIFIER_LENGTH];
    } value;
} ASTNode;

#define PRINT_ASTNODE(node)                                                                        \
    printf("ASTNode Information\n-------------------\n");                                          \
    printf("TokenType: %s\nLeft: %d\nMiddle: %d\nRight: %d\n", tokenStrings[node->ttype],          \
           (int)node->left, (int)node->mid, (int)node->right);                                     \
    printf("Is RValue: %s\n", node->is_rvalue ? "true" : "false");                                 \
    printf("# of Func Call Args: %llu\n", node->num_args);                                         \
    printf("Value (int): %lld\n", node->value.number_value);                                       \
    printf("Value (str): %s\n", node->value.symbol_name);

ASTNode* create_ast_node(TokenType ttype, ASTNode* left, ASTNode* mid, ASTNode* right, Type type,
                         char* symbol_name);
void add_position_info(ASTNode* dest, position p);
ASTNode* create_ast_nonidentifier_leaf(TokenType ttype, Type type);
ASTNode* create_ast_identifier_leaf(TokenType ttype, char* symbol_name);
ASTNode* create_unary_ast_node(TokenType ttype, ASTNode* child, Type type, char* symbol_name);
void ast_debug_level_order(ASTNode* root, LogLevel log_level);
void free_ast_node(ASTNode* root);

#endif /* TREE */
