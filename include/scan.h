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
typedef enum
{
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
    // Literals
    T_INTEGER_LITERAL,
    // Types
    T_INT,
    // Assignment
    T_ASSIGN,
    // Keywords
    T_PRINT,
    // Miscellaneous
    T_SEMICOLON,
    T_IDENTIFIER,
    T_LVALUE_IDENTIFIER,
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
#define TTS_INTEGER_LITERAL "integer literal"
#define TTS_INT "int"
#define TTS_ASSIGN "="
#define TTS_PRINT "print"
#define TTS_SEMICOLON ";"
#define TTS_IDENTIFIER "identifier"
#define TTS_LVALUE_IDENTIFIER "lvalue identifier"

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
                               TTS_INTEGER_LITERAL,
                               TTS_INT,
                               TTS_ASSIGN,
                               TTS_PRINT,
                               TTS_SEMICOLON,
                               TTS_IDENTIFIER,
                               TTS_LVALUE_IDENTIFIER};

/**
 * @brief Macro to determine if a TokenType is associated with a terminal AST Node
 */

#define TOKENTYPE_IS_TERMINAL(type)                                                                \
    (/*Literals*/ (type >= T_INTEGER_LITERAL && type <= T_INTEGER_LITERAL) ||                      \
     /*Types*/ (type >= T_INT && type <= T_INT) ||                                                 \
     /*Miscellaneous*/ (type >= T_SEMICOLON && type <= T_LVALUE_IDENTIFIER))

/**
 * @brief Macro to determine if a TokenType is associated with a binary arithmetic operation
 */
#define TOKENTYPE_IS_BINARY_ARITHMETIC(type) (type >= T_PLUS && type <= T_EXPONENT)

#define TOKENTYPE_IS_LITERAL(type) (type >= T_INTEGER_LITERAL && type <= T_INTEGER_LITERAL)

#define TOKENTYPE_IS_IDENTIFIER(type) (type >= T_IDENTIFIER && type <= T_LVALUE_IDENTIFIER)

/**Purple identifiers can be a maximum of 255 bytes in length*/
#define D_MAX_IDENTIFIER_LENGTH 255

/**
 * @brief Structure containing information about individual scannable tokens
 */
typedef struct Token {
    /**Type of token*/
    TokenType type;
    /**Value of token*/
    union {
        /**Value of integer token*/
        int int_value;
        /**Name of identifier token*/
        char symbol_name[D_MAX_IDENTIFIER_LENGTH];
    } value;
} Token;

bool scan(Token* t);

#endif /* SCAN_H */
