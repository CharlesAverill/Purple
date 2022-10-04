/**
 * @file hash.h
 * @author Charles Averill
 * @brief Function headers and definitions for hashing algorithms
 * @date 17-Sep-2022
 */

#ifndef HASH_H
#define HASH_H

/**Offset basis for FNV-1 algorithm*/
#define FNV_OFFSET_BASIS 0xCBF29CE484222325
/**Prime number for FNV-1 algorithm*/
#define FNV_PRIME 0x100000001B3

unsigned long int FNV_1(char* str);

#endif /* HASH_H */
