/**
 * @file
 * @author CharlesAverill
 * @date   05-Oct-2021
 * @brief Lexical Scanner function headers
*/

#ifndef SCAN_H
#define SCAN_H

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/logging.h"

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
    // Keywords
    T_PRINT,
    // Miscellaneous
    T_SEMICOLON,
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
#define TTS_PRINT "print"
#define TTS_SEMICOLON ";"

/**
 * @brief Token string equivalents
 */
static char* tokenStrings[] = {
    TTS_EOF,      TTS_PLUS, TTS_MINUS, TTS_STAR, TTS_SLASH, TTS_EXPONENT,        TTS_EQ,
    TTS_NEQ,      TTS_LT,   TTS_GT,    TTS_LE,   TTS_GE,    TTS_INTEGER_LITERAL, TTS_PRINT,
    TTS_SEMICOLON};

/**
 * @brief Macro to determine if a TokenType is associated with a terminal AST Node
 */
#define TOKENTYPE_IS_TERMINAL(type) (type >= T_INTEGER_LITERAL && type <= T_INTEGER_LITERAL)

/**
 * @brief Macro to determine if a TokenType is associated with a binary arithmetic operation
 */
#define TOKENTYPE_IS_BINARY_ARITHMETIC(type) (type >= T_PLUS && type <= T_EXPONENT)

/**
 * @brief Structure containing information about individual scannable tokens
 */
typedef struct Token {
    /**Type of token*/
    TokenType type;
    /**Value of integer token*/
    int value;
} Token;

bool scan(Token* t);

#endif /* SCAN_H */
