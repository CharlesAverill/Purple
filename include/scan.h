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

#include "data.h"
#include "errors_warnings.h"

typedef enum {
    T_EOF,
    // Arithmetic Operators
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    // Comparison Operators
    T_EQ,
    T_NEQ,
    T_LT,
    T_GT,
    T_LE,
    T_GE,
    // Literals
    T_INTEGER_LITERAL
} TokenType;

/**
 * Token string equivalents
 */
static char* token_strings[] = {"EOF",      "+",     "-",          "*",
                                "/",        "==",    "!=",         "<",
                                ">",        "<=",    ">=",         "integer literal",
                                ";",        "=",     "(",          ")",
                                "{",        "}",     "identifier", "int",
                                "as",       "if",    "else",       "for",
                                "print",    "while", "with",       "LValue Identifier",
                                "AST Glue", "Scope"};

typedef struct Token {
    TokenType type;
    int value;
} Token;

bool scan(Token* t);

#endif /* SCAN_H */
