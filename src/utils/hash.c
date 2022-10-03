/**
 * @file hash.c
 * @author Charles Averill
 * @brief Logic for hashing functions
 * @date 17-Sep-2022
 */

#include "utils/hash.h"

/**
 * @brief FNV-1 String hashing algorithm as defined here: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash
 * 
 * @param str String to be hashed
 * @return unsigned long int Hash value
 */
unsigned long int FNV_1(char* str)
{
    unsigned long int hash = FNV_OFFSET_BASIS;
    char c;

    while ((c = *str++)) {
        hash *= FNV_PRIME;
        hash ^= c;
    }

    return hash;
}
