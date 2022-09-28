/**
 * @file
 * @author CharlesAverill
 * @date   05-Oct-2021
 * @brief Lexical Scanner function headers
*/

#ifndef SCAN_H
#define SCAN_H

#include <stdbool.h>

#include "translate/symbol_table.h"

/**
 * @brief Types of scannable tokens
 */
typedef enum {
    T_EOF,
    // Arithmetic Operators
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_EXPONENT,
    // Comparison Operators
    T_EQ,
    T_NEQ,
    T_LT,
    T_GT,
    T_LE,
    T_GE,
    // Logical Operators
    T_AND,
    T_OR,
    T_XOR,
    T_NAND,
    T_NOR,
    T_XNOR,
    // Literals
    T_INTEGER_LITERAL,
    T_CHAR_LITERAL,
    T_LONG_LITERAL,
    T_TRUE,
    T_FALSE,
    // Types
    T_BOOL,
    T_CHAR,
    T_INT,
    T_LONG,
    // Assignment
    T_ASSIGN,
    // Keywords
    T_PRINT,
    T_IF,
    T_ELSE,
    T_WHILE,
    T_FOR,
    // Miscellaneous
    T_SEMICOLON,
    T_LEFT_PAREN,
    T_RIGHT_PAREN,
    T_LEFT_BRACE,
    T_RIGHT_BRACE,
    T_IDENTIFIER,
    T_LVALUE_IDENTIFIER,
    T_AST_GLUE,
} TokenType;

#define TTS_EOF "EOF"
#define TTS_PLUS "+"
#define TTS_MINUS "-"
#define TTS_STAR "*"
#define TTS_SLASH "/"
#define TTS_EXPONENT "**"
#define TTS_EQ "=="
#define TTS_NEQ "!="
#define TTS_LT "<"
#define TTS_GT ">"
#define TTS_LE "<="
#define TTS_GE ">="
#define TTS_AND "and"
#define TTS_OR "or"
#define TTS_XOR "xor"
#define TTS_NAND "nand"
#define TTS_NOR "nor"
#define TTS_XNOR "xnor"
#define TTS_INTEGER_LITERAL "integer literal"
#define TTS_CHAR_LITERAL "character literal"
#define TTS_LONG_LITERAL "long literal"
#define TTS_TRUE "true"
#define TTS_FALSE "false"
#define TTS_BOOL "bool"
#define TTS_CHAR "char"
#define TTS_INT "int"
#define TTS_LONG "long"
#define TTS_ASSIGN "="
#define TTS_PRINT "print"
#define TTS_IF "if"
#define TTS_ELSE "else"
#define TTS_WHILE "while"
#define TTS_FOR "for"
#define TTS_SEMICOLON ";"
#define TTS_LEFT_PAREN "("
#define TTS_RIGHT_PAREN ")"
#define TTS_LEFT_BRACE "{"
#define TTS_RIGHT_BRACE "}"
#define TTS_IDENTIFIER "identifier"
#define TTS_LVALUE_IDENTIFIER "lvalue identifier"
#define TTS_AST_GLUE "ast glue"

#define NUMBER_LITERAL_BASE_PREFIX '0'
#define NUMBER_LITERAL_BIN_PREFIX 'b'
#define NUMBER_LITERAL_OCT_PREFIX 'o'
#define NUMBER_LITERAL_HEX_PREFIX 'x'
#define NUMBER_LITERAL_LONG_SUFFIX 'L'

#define NUMBER_LITERAL_SPACING_SEPARATOR '\''
#define NUMBER_LITERAL_BASE_SEPARATOR '#'

/**
 * @brief Token string equivalents
 */
static char* tokenStrings[] = {TTS_EOF,
                               TTS_PLUS,
                               TTS_MINUS,
                               TTS_STAR,
                               TTS_SLASH,
                               TTS_EXPONENT,
                               TTS_EQ,
                               TTS_NEQ,
                               TTS_LT,
                               TTS_GT,
                               TTS_LE,
                               TTS_GE,
                               TTS_AND,
                               TTS_OR,
                               TTS_XOR,
                               TTS_NAND,
                               TTS_NOR,
                               TTS_XNOR,
                               TTS_INTEGER_LITERAL,
                               TTS_CHAR_LITERAL,
                               TTS_LONG_LITERAL,
                               TTS_TRUE,
                               TTS_FALSE,
                               TTS_BOOL,
                               TTS_CHAR,
                               TTS_INT,
                               TTS_LONG,
                               TTS_ASSIGN,
                               TTS_PRINT,
                               TTS_IF,
                               TTS_ELSE,
                               TTS_WHILE,
                               TTS_FOR,
                               TTS_SEMICOLON,
                               TTS_LEFT_PAREN,
                               TTS_RIGHT_PAREN,
                               TTS_LEFT_BRACE,
                               TTS_RIGHT_BRACE,
                               TTS_IDENTIFIER,
                               TTS_LVALUE_IDENTIFIER,
                               TTS_AST_GLUE};

/**
 * @brief Determines if a TokenType is associated with a binary arithmetic operation
 */
#define TOKENTYPE_IS_BINARY_ARITHMETIC(type) (type >= T_PLUS && type <= T_EXPONENT)

/**
 * @brief Determines if a TokenType is associated with a type keyword
 */
#define TOKENTYPE_IS_TYPE(type) (type >= T_BOOL && type <= T_LONG)

/**
 * @brief Determines if a TokenType is associated with a literal value
 */
#define TOKENTYPE_IS_LITERAL(type) (type >= T_INTEGER_LITERAL && type <= T_FALSE)

/**
 * @brief Determines if a TokenType is associated with a boolean literal value
 */
#define TOKENTYPE_IS_BOOL_LITERAL(type) (type >= T_TRUE && type <= T_FALSE)

/**
 * @brief Determines if a TokenType is associated with an identifier
 */
#define TOKENTYPE_IS_IDENTIFIER(type) (type >= T_IDENTIFIER && type <= T_LVALUE_IDENTIFIER)

/**
 * @brief Determines if a TokenType is associated with a comparison operator
 */
#define TOKENTYPE_IS_COMPARATOR(type) (type >= T_EQ && type <= T_GE)

/**
 * @brief Determines if a TokenType is associated with a logical operator
 * 
 */
#define TOKENTYPE_IS_LOGICAL_OPERATOR(type) (type >= T_AND && type <= T_XNOR)

/**
 * @brief Number literals can have a max value of 2^63 - 1 and a min value of -2^63
 */
#define number_literal_type long long int

/**
 * @brief len(str(2^63)) = 19
 */
#define MAX_NUMBER_LITERAL_DIGITS 19

/**
 * @brief Structure containing information about individual scannable tokens
 */
typedef struct Token {
    /**Type of token*/
    TokenType type;
    /**Value of token*/
    union {
        /**Value of integer token*/
        Number number_value;
        /**Name of identifier token*/
        char symbol_name[D_MAX_IDENTIFIER_LENGTH];
    } value;
} Token;

bool scan(Token* t);

#endif /* SCAN_H */
