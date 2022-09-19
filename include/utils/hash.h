/**
 * @file hash.h
 * @author Charles Averill
 * @brief Function headers and definitions for hashing algorithms
 * @date 17-Sep-2022
 */

#ifndef HASH_H
#define HASH_H

#define FNV_OFFSET_BASIS 0xCBF29CE484222325
#define FNV_PRIME 0x100000001B3

unsigned long int FNV_1(char* str);

#endif /* HASH_H */
