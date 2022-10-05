/**
 * @file function.h
 * @author Charles Averill
 * @brief Struct definitions and macros for functions
 * @date 05-Oct-2022
 */

#ifndef FUNCTION_H
#define FUNCTION_H

#include "scan.h"

/**
 * @brief Container for function information
 */
typedef struct Function {
    /**Return type of this function*/
    TokenType return_type;
    /**Array of input types for this function*/
    TokenType* parameter_types;
} Function;

/**
 * @brief Generates a Function struct given an output type and an array of input types
 */
#define FUNCTION_FROM_OUT_IN(output_type, input_array)                                             \
    (Function)                                                                                     \
    {                                                                                              \
        .return_type = output_type, .parameter_types = input_array                                 \
    }

#endif /* FUNCTION_H */
