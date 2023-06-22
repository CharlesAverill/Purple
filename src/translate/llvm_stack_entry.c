/**
 * @file llvm_stack_entry.c
 * @author Charles Averill
 * @brief 
 * @date 15-Sep-2022
 */

#include <stdlib.h>

#include "translate/llvm_stack_entry.h"
#include "utils/logging.h"

/**
 * @brief Initialize a stack entry linked list
 * 
 * @param head Head of linked list
 */
void initialize_stack_entry_linked_list(LLVMStackEntryNode** head)
{
    *head = NULL;
}

/**
 * @brief Add a stack entry to a stack entry linked list
 * 
 * @param head Head of linked list
 * @param register_index Register number to push onto list
 */
void prepend_stack_entry_linked_list(LLVMStackEntryNode** head, LLVMStackEntryNode* reg)
{
    LLVMStackEntryNode* copy = (LLVMStackEntryNode*)malloc(sizeof(LLVMStackEntryNode));
    copy->reg = reg->reg;
    copy->align_bytes = reg->align_bytes;

    if (*head == NULL) {
        copy->next = NULL;
        *head = copy;
    } else {
        reg->next = *head;
        *head = copy;
    }
}

/**
 * @brief Pop and free a stack entry from a Stack Entry Node linked list and get its type
 * 
 * @param head Head of linked list to pop from
 * @return type_register Register number from first entry in linked list
 */
type_register pop_stack_entry_linked_list(LLVMStackEntryNode** head)
{
    LLVMStackEntryNode* temp;

    if (*head == NULL) {
        fatal(RC_COMPILER_ERROR, "Tried to pop from an LLVMStackEntryNode List when it was empty");
    }

    if ((**head).reg.value_type == LLVMVALUETYPE_NONE) {
        purple_log(LOG_WARNING, "pop_stack_entry_linked_list was given a corrupted LLVMValue");
        *head = NULL;
        return 0;
    }

    type_register out = (*head)->reg.value.virtual_register_index;
    temp = (*head)->next;
    purple_log(LOG_DEBUG, "Freeing *head in pop_stack_entry_linked_list");
    free(*head);
    *head = temp;

    return out;
}

/**
 * @brief Free memory used by LLVMStackEntryNode linked lists
 * 
 * @param head Head of list to free
 */
void free_llvm_stack_entry_node_list(LLVMStackEntryNode** head)
{
    LLVMStackEntryNode* current = *head;
    LLVMStackEntryNode* prev;
    while (current) {
        prev = current;
        current = current->next;
        purple_log(LOG_DEBUG, "Freeing LLVMStackEntryNode at %p in free_llvm_stack_entry_node_list",
                   prev);
        free(prev);
    }
    *head = NULL;
}
