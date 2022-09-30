/**
 * @file number.h
 * @author Charles Averill
 * @brief Definitions and function headers for the internal "Number" type
 * @date 12-Sep-2022
 */

#ifndef NUMBER_H
#define NUMBER_H

#include <limits.h>

/**
 * @brief Types of numbers supported by Purple
 */
typedef enum {
    NT_INT1,
    NT_INT8,
    NT_INT16,
    NT_INT32,
    NT_INT64,
} NumberType;

/**
 * @brief Size of each NumberType
 */
static const int numberTypeByteSizes[] = {
    1, 1, 2, 4, 8,
};

/**
 * @brief Max value of each NumberType
 */
static const unsigned long long int numberTypeMaxValues[] = {1, CHAR_MAX, SHRT_MAX, INT_MAX,
                                                             LONG_MAX};

/**
 * @brief Format strings for each data type
 */
static const char* numberTypeFormatStrings[] = {"%d", "%d", "%d", "%d", "%ld"};

/**
 * @brief Container for various kinds of number data
 */
typedef struct Number {
    /**Data type of number*/
    NumberType type;
    /**Value of number*/
    long long int value;
} Number;

/**
 * @brief Generates a Number struct with type INT1
 */
#define NUMBER_BOOL(v)                                                                             \
    (Number)                                                                                       \
    {                                                                                              \
        .type = NT_INT1, .value = v                                                                \
    }

/**
 * @brief Generates a Number struct with type INT8
 */
#define NUMBER_CHAR(v)                                                                             \
    (Number)                                                                                       \
    {                                                                                              \
        .type = NT_INT8, .value = v                                                                \
    }

/**
 * @brief Generates a Number struct with type INT16
 * 
 */
#define NUMBER_SHORT(v)                                                                            \
    (Number)                                                                                       \
    {                                                                                              \
        .type = NT_INT16, .value = v                                                               \
    }

/**
 * @brief Generates a Number struct with type INT32
 */
#define NUMBER_INT(v)                                                                              \
    (Number)                                                                                       \
    {                                                                                              \
        .type = NT_INT32, .value = v                                                               \
    }

/**
 * @brief Generates a Number struct with type INT64
 */
#define NUMBER_LONG(v)                                                                             \
    (Number)                                                                                       \
    {                                                                                              \
        .type = NT_INT64, .value = v                                                               \
    }

NumberType token_type_to_number_type(int token_type);

#endif /* NUMBER_H */
