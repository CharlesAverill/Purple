/**
 * @file type.h
 * @author Charles Averill
 * @brief Function headers and defines for 
 * @date 28-Sep-2022
 */

#ifndef TYPE_H
#define TYPE_H

#include "types/number.h"

/**
 * @brief Types of types
 */
typedef enum
{
    TT_VOID,
    TT_NUMBER
} TypeType;

/**
 * @brief Container for type data
 */
typedef struct Type {
    /**Type of this type*/
    TypeType type;
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
        .type = TT_VOID                                                                            \
    }

/**
 * @brief Generates a number type struct given a Number
 */
#define TYPE_NUMBER_VAL(n)                                                                         \
    (Type)                                                                                         \
    {                                                                                              \
        .type = TT_NUMBER, .value.number = n                                                       \
    }

/**
 * @brief Generates a number type struct given a NumberType
 */
#define TYPE_NUMBER(nt)                                                                            \
    (Type)                                                                                         \
    {                                                                                              \
        .type = TT_NUMBER, .value.number = (Number)                                                \
        {                                                                                          \
            .type = nt                                                                             \
        }                                                                                          \
    }

#endif /* TYPE_H */
