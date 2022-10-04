/**
 * @file type.h
 * @author Charles Averill
 * @brief Function headers and defines for 
 * @date 28-Sep-2022
 */

#ifndef TYPE_H
#define TYPE_H

#include "scan.h"
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

#define TYPE_NUMBER_TOKEN(ttype)                                                                   \
    (Type)                                                                                         \
    {                                                                                              \
        .type = ttype, .value.number = NUMBER_FROM_TYPE_VAL(token_type_to_number_type(ttype), 0)   \
    }

/**
 * @brief Generates a number type struct given a Number
 */
#define TYPE_NUMBER_VAL(n)                                                                         \
    (Type)                                                                                         \
    {                                                                                              \
        .type = number_to_token_type(n), .value.number = n                                         \
    }

/**
 * @brief Generates a number type struct given a NumberType
 */
#define TYPE_NUMBER(nt)                                                                            \
    (Type)                                                                                         \
    {                                                                                              \
        .type = number_to_token_type((Number){.type = nt}), .value.number = (Number)               \
        {                                                                                          \
            .type = nt                                                                             \
        }                                                                                          \
    }

#define TOKENTYPE_IS_NUMBER_TYPE(tt)                                                               \
    ((tt >= T_BOOL && tt <= T_LONG) || (T_TRUE <= tt && tt <= T_LONG_LITERAL))

#endif /* TYPE_H */
