/**
 * @file llvm_stack_entry.h
 * @author Charles Averill
 * @brief 
 * @date 15-Sep-2022
 */

#ifndef LLVM_STACK_ENTRY_H
#define LLVM_STACK_ENTRY_H

#include "types/number.h"

/**
 * @brief A too-big data type for register indices in case of stress testing
 */
#define type_register unsigned long long int

/**
 * @brief A too-big data type for label indices in case of stress testing
 */
#define type_label unsigned long long int

/**
 * @brief Node of a linked list containing information about required stack allocation for a piece of data. 
 * Used in the symbol table linked list.
 * Also used as a general-purpose linked list of virtual registers with loaded values.
 */
typedef struct LLVMStackEntryNode {
    /**Register number of the current node*/
    type_register reg;
    /**Pointer depth of the current node*/
    int pointer_depth;
    /**Data type of the current node*/
    NumberType type;
    /**Number of bytes to align stack to for this data*/
    int align_bytes;
    /**Next node in linked list*/
    struct LLVMStackEntryNode* next;
} LLVMStackEntryNode;

void initialize_stack_entry_linked_list(LLVMStackEntryNode** head);
void prepend_stack_entry_linked_list(LLVMStackEntryNode** head, type_register register_index);
type_register pop_stack_entry_linked_list(LLVMStackEntryNode** head);
void free_llvm_stack_entry_node_list(LLVMStackEntryNode* head);

#endif /* LLVM_STACK_ENTRY_H */
