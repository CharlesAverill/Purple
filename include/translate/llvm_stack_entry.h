/**
 * @file llvm_stack_entry.h
 * @author Charles Averill
 * @brief 
 * @date 15-Sep-2022
 */

#ifndef LLVM_STACK_ENTRY_H
#define LLVM_STACK_ENTRY_H

#include "translate/llvm.h"
#include "types/number.h"

/**
 * @brief Node of a linked list containing information about required stack allocation for a piece of data. 
 * Used in the symbol table linked list.
 * Also used as a general-purpose linked list of virtual registers with loaded values.
 */
typedef struct LLVMStackEntryNode {
    /**Register number of the current node*/
    LLVMValue reg;
    /**Number of bytes to align stack to for this data*/
    int align_bytes;
    /**Next node in linked list*/
    struct LLVMStackEntryNode* next;
} LLVMStackEntryNode;

/**
 * @brief Head node of linked list containing register indices that are free to have values stored in them
 */
extern LLVMStackEntryNode* freeVirtualRegistersHead;

void initialize_stack_entry_linked_list(LLVMStackEntryNode** head);
void prepend_stack_entry_linked_list(LLVMStackEntryNode** head, LLVMStackEntryNode* reg);
LLVMValue pop_stack_entry_linked_list(LLVMStackEntryNode** head);
void free_llvm_stack_entry_node_list(LLVMStackEntryNode* head);

bool llvm_stack_allocation(LLVMStackEntryNode* stack_entries);

#endif /* LLVM_STACK_ENTRY_H */
