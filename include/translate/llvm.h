/**
 * @file llvm.h
 * @author Charles Averill
 * @brief Function headers for LLVM-IR emission
 * @date 10-Sep-2022
 */

#ifndef LLVM_H
#define LLVM_H

#include "scan.h"
#include "types/number.h"

void llvm_preamble();
void llvm_postamble();

int llvm_binary_arithmetic(TokenType operation, int left_virtual_register,
                           int right_virtual_register);
int llvm_load_number(Number value);
void increment_local_virtual_register_number(void);

#endif /* LLVM_H */
