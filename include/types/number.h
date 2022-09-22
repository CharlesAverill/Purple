/**
 * @file number.h
 * @author Charles Averill
 * @brief Definitions and function headers for the internal "Number" type
 * @date 12-Sep-2022
 */

#ifndef NUMBER
#define NUMBER

/**
 * @brief Types of numbers supported by Purple
 */
typedef enum {
    NT_INT1,
    NT_INT32,
} NumberType;

/**
 * @brief Size of each NumberType
 */
static const int numberTypeByteSizes[] = {
    1,
    4,
};

/**
 * @brief Format strings for each data type
 */
static const char* numberTypeFormatStrings[] = {
    "%d",
    "%d",
};

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
#define NUMBER_INT32(v)                                                                            \
    (Number) { .type = NT_INT32, .value.int_value = v }

/**
 * @brief Generates a Number struct with type INT1
 */
#define NUMBER_INT1(v)                                                                             \
    (Number) { .type = NT_INT1, .value.int_value = v }

NumberType token_type_to_number_type(int token_type);

#endif /* NUMBER */
