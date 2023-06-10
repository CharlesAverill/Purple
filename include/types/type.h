/**
 * @file type.h
 * @author Charles Averill
 * @brief Function headers and defines for 
 * @date 28-Sep-2022
 */

#ifndef TYPE
#define TYPE

#include "scan.h"
#include "types/function.h"
#include "types/number.h"

/**
 * @brief Container for type data
 */
typedef struct Type {
    /**Type of this type*/
    TokenType token_type;
    /**Whether or not this is the type of a function*/
    bool is_function;
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
        .token_type = T_VOID, .is_function = false                                                 \
    }

/**
 * @brief Generates a number type struct given a TokenType
 */
#define TYPE_NUMBER_FROM_NUMBERTYPE_FROM_TOKEN(ttype)                                              \
    (Type)                                                                                         \
    {                                                                                              \
        .token_type = ttype,                                                                       \
        .value.number = NUMBER_FROM_TYPE_VAL(token_type_to_number_type(ttype), 0),                 \
        .is_function = false                                                                       \
    }

/**
 * @brief Generates a number type struct given a Number
 */
#define TYPE_NUMBER_FROM_NUMBERTYPE_FROM_NUMBER(n)                                                 \
    (Type)                                                                                         \
    {                                                                                              \
        .token_type = number_to_token_type(n), .value.number = n, .is_function = false             \
    }

/**
 * @brief Generates a number type struct given a NumberType
 */
#define TYPE_NUMBER_FROM_NUMBERTYPE(nt)                                                            \
    (Type)                                                                                         \
    {                                                                                              \
        .token_type = number_to_token_type((Number){.token_type = nt}), .is_function = false,      \
        .value.number = NUMBER_FROM_TYPE_VAL(nt, 0)                                                \
    }

#define TYPE_FUNCTION(out, input_list, num_inputs)                                                 \
    (Type)                                                                                         \
    {                                                                                              \
        .token_type = T_FUNCTION_DECLARATION, .is_function = true,                                 \
        .value.function = FUNCTION_FROM_OUT_IN(out, input_list, num_inputs)                        \
    }

#endif /* TYPE */
