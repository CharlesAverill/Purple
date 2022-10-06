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
 * @brief Container for function parameter information
 */
typedef struct FunctionParameter {
    /**Type of this parameter*/
    TokenType parameter_type;
    /**Name of this parameter*/
    // TODO : fix this MAX_IDENTIFIER_LENGTH drop-in
    char parameter_name[256];
} FunctionParameter;

/**
 * @brief Container for function information
 */
typedef struct Function {
    /**Return type of this function*/
    TokenType return_type;
    /**Array of input types for this function*/
    FunctionParameter* parameters;
    /**Number of function parameters*/
    unsigned long long int num_parameters;
} Function;

/**
 * @brief Generates a Function struct given an output type and a linked list of input types
 */
#define FUNCTION_FROM_OUT_IN(output_type, input_list, num_inputs)                                  \
    (Function)                                                                                     \
    {                                                                                              \
        .return_type = output_type, .parameters = input_list, .num_parameters = num_inputs         \
    }

#endif /* FUNCTION_H */
