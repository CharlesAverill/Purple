/**
 * @file
 * @author CharlesAverill
 * @date   05-Oct-2021
 * @brief Lexical Scanner function headers
*/

#ifndef SCAN_H
#define SCAN_H

#include <stdbool.h>

#include "types/identifier.h"
#include "types/number.h"

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
    // Logical Operators
    T_AND,
    T_OR,
    T_XOR,
    T_NAND,
    T_NOR,
    T_XNOR,
    // Pointer operators
    T_AMPERSAND,
    T_DEREFERENCE,
    // Literals
    T_TRUE,
    T_FALSE,
    T_BYTE_LITERAL,
    T_CHAR_LITERAL,
    T_SHORT_LITERAL,
    T_INTEGER_LITERAL,
    T_LONG_LITERAL,
    // Types
    T_VOID,
    T_BOOL,
    T_BYTE,
    T_CHAR,
    T_SHORT,
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
    T_RETURN,
    // Miscellaneous
    T_SEMICOLON,
    T_LEFT_PAREN,
    T_RIGHT_PAREN,
    T_LEFT_BRACE,
    T_RIGHT_BRACE,
    T_IDENTIFIER,
    // T_LVALUE_IDENTIFIER,
    T_AST_GLUE,
    T_FUNCTION_DECLARATION,
    T_FUNCTION_CALL,
    // Enum End value
    TOKENTYPE_MAX
} TokenType;

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
static char* tokenStrings[] = {
    "EOF", "+", "-", "*", "/", "pow", "==", "!=", "<", ">", "<=", ">=", "and", "or", "xor", "nand",
    "nor", "xnor", "&", "*", "true", "false", "byte literal", "character literal", "short literal",
    "integer literal", "long literal", "void", "bool", "byte", "char", "short", "int", "long", "=",
    "print", "if", "else", "while", "for", "return", ";", "(", ")", "{", "}", "identifier",
    //    "lvalue identifier",
    "ast glue", "function", "function call", "TOKENTYPE_MAX"};

/**
 * @brief Determines if a TokenType is associated with a binary arithmetic operation
 */
#define TOKENTYPE_IS_BINARY_ARITHMETIC(type) (type >= T_PLUS && type <= T_EXPONENT)

/**
 * @brief Determines if a TokenType is associated with a type keyword
 */
#define TOKENTYPE_IS_TYPE(type) (type >= T_VOID && type <= T_LONG)

/**
 * @brief Determines if a TokenType is associated with a number type keyword or a number literal
 */
#define TOKENTYPE_IS_NUMBER_TYPE(tt)                                                               \
    ((tt >= T_BOOL && tt <= T_LONG) || (T_TRUE <= tt && tt <= T_LONG_LITERAL))

/**
 * @brief Determines if a TokenType is associated with a literal value
 */
#define TOKENTYPE_IS_LITERAL(type) (type >= T_TRUE && type <= T_LONG_LITERAL)

/**
 * @brief Determines if a TokenType is associated with a boolean literal value
 */
#define TOKENTYPE_IS_BOOL_LITERAL(type) (type >= T_TRUE && type <= T_FALSE)

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
 * @brief Structure containing information about a Token's position in the input
 */
typedef struct position {
    /**Name of file*/
    char filename[256];
    /**Line number in file*/
    int line_number;
    /**Character number in line*/
    int char_number;
} position;

/**
 * @brief Structure containing information about individual scannable tokens
 */
typedef struct Token {
    /**Type of Token*/
    TokenType type;
    /**Position of Token*/
    position pos;
    /**Value of Token*/
    union {
        /**Value of integer Token*/
        Number number_value;
        /**Name of identifier Token*/
        char symbol_name[MAX_IDENTIFIER_LENGTH];
    } value;
} Token;

char next(void);
void put_back_into_stream(char c);
bool scan();

#endif /* SCAN_H */
