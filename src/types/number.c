/**
 * @file number.c
 * @author Charles Averill
 * @brief Functions for dealing with numbers internally
 * @date 12-Sep-2022
 */

#include <math.h>
#include <stdio.h>

#include "scan.h"
#include "types/number.h"

/**
 * @brief Convert a TokenType to a NumberType
 * 
 * @param token_type TokenType to convert
 * @return NumberType Converted TokenType 
 */
NumberType token_type_to_number_type(int token_type)
{
    switch (token_type) {
    case T_BOOL:
    case T_TRUE:
    case T_FALSE:
        return NT_INT1;
    case T_BYTE:
    case T_BYTE_LITERAL:
    case T_CHAR:
    case T_CHAR_LITERAL:
        return NT_INT8;
    case T_SHORT:
    case T_SHORT_LITERAL:
        return NT_INT16;
    case T_INT:
    case T_INTEGER_LITERAL:
        return NT_INT32;
    case T_LONG:
    case T_LONG_LITERAL:
        return NT_INT64;
    }

    return -1;
}

int number_to_token_type(Number number)
{
    switch (number.number_type) {
    case NT_INT1:
        return number.value ? T_TRUE : T_FALSE;
    case NT_INT8:
        return T_BYTE_LITERAL;
    case NT_INT16:
        return T_SHORT_LITERAL;
    case NT_INT32:
        return T_INTEGER_LITERAL;
    case NT_INT64:
        return T_LONG_LITERAL;
    }
}

/**
 * @brief Finds the maximum NumberType possible for a given value
 * 
 * @param value         Value to check
 * @return NumberType   Corresponding NumberType, or -1 if none was found
 */
NumberType max_numbertype_for_val(long long int value)
{
    for (int i = NT_INT64; i >= NT_INT1; i--) {
        if (value >= (int)pow(2, numberTypeByteSizes[i]) - 1) {
            return i;
        }
    }
    return -1;
}
