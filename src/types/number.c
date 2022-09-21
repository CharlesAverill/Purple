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
    case T_INTEGER_LITERAL:
        return NT_INT32;
    case T_TRUE:
    case T_FALSE:
        return NT_INT1;
    }
    return -1;
}