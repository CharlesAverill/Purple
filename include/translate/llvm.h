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

/**
 * @brief Node of a linked list containing information about required stack allocation for a function
 */
typedef struct LLVMStackEntryNode {
    /**Register number of the current node*/
    unsigned long long int reg;
    /**Data type of the current node*/
    NumberType type;
    /**Number of bytes to align stack to for this data*/
    int align_bytes;
    /**Next node in linked list*/
    struct LLVMStackEntryNode* next;
} LLVMStackEntryNode;

/**
 * @brief LLVM-IR representations of data types
 */
static const char* numberTypeLLVMReprs[] = {
    "i32",
};

void llvm_preamble();
void llvm_postamble();

void llvm_stack_allocation(LLVMStackEntryNode* stack_entries);

int llvm_binary_arithmetic(TokenType operation, int left_virtual_register,
                           int right_virtual_register);
int llvm_load_constant(Number value);
unsigned long long int get_next_local_virtual_register(void);

#endif /* LLVM_H */
