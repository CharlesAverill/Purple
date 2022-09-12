/**
 * @file number.h
 * @author Charles Averill
 * @brief Definitions and function headers for the internal "Number" type
 * @date 12-Sep-2022
 */

#ifndef NUMBER_H
#define NUMBER_H

/**
 * @brief Types of numbers supported by Purple
 */
typedef enum {
    NT_INT32,
} NumberType;

/**
 * @brief Container for various kinds of number data
 */
typedef struct Number {
    /**Data type of number*/
    NumberType type;
    /**Value of number*/
    union {
        int int_value;
    } value;
} Number;

/**
 * @brief Generates a Number struct with type INT32
 */
#define NUMBER_INT32(v) (Number){.type=NT_INT32, .value.int_value = v}

#endif /* NUMBER_H */
