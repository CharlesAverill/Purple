/**
 * @file type.h
 * @author Charles Averill
 * @brief Function headers and defines for 
 * @date 28-Sep-2022
 */

#ifndef TYPE_H
#define TYPE_H

#include "scan.h"
#include "types/function.h"
#include "types/number.h"

/**
 * @brief Container for type data
 */
typedef struct Type {
    /**Type of this type*/
    TokenType type;
    /**Value of this type*/
    union {
        Number number;
        Function function;
    } value;
} Type;

/**
 * @brief Generates a void type struct
 */
#define TYPE_VOID                                                                                  \
    (Type)                                                                                         \
    {                                                                                              \
        .type = T_VOID                                                                             \
    }

/**
 * @brief Generates a number type struct given a TokenType
 */
#define TYPE_NUMBER_FROM_NUMBERTYPE_FROM_TOKEN(ttype)                                              \
    (Type)                                                                                         \
    {                                                                                              \
        .type = ttype, .value.number = NUMBER_FROM_TYPE_VAL(token_type_to_number_type(ttype), 0)   \
    }

/**
 * @brief Generates a number type struct given a Number
 */
#define TYPE_NUMBER_FROM_NUMBERTYPE_FROM_NUMBER(n)                                                 \
    (Type)                                                                                         \
    {                                                                                              \
        .type = number_to_token_type(n), .value.number = n                                         \
    }

/**
 * @brief Generates a number type struct given a NumberType
 */
#define TYPE_NUMBER_FROM_NUMBERTYPE(nt)                                                            \
    (Type)                                                                                         \
    {                                                                                              \
        .type = number_to_token_type((Number){.type = nt}),                                        \
        .value.number = NUMBER_FROM_TYPE_VAL(nt, 0)                                                \
    }

#define TYPE_FUNCTION(out, ...)                                                                    \
    (Type)                                                                                         \
    {                                                                                              \
        .type = T_FUNCTION,                                                                        \
        .value.function = FUNCTION_FROM_OUT_IN(out, (TokenType[]){__VA_ARGS__})                    \
    }

#endif /* TYPE_H */
