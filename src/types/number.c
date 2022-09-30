/**
 * @file number.c
 * @author Charles Averill
 * @brief Functions for dealing with numbers internally
 * @date 12-Sep-2022
 */

#include "types/number.h"
#include "scan.h"

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
    case T_CHAR:
    case T_CHAR_LITERAL:
        return NT_INT8;
    case T_SHORT:
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