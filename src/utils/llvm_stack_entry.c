/**
 * @file llvm_stack_entry.c
 * @author Charles Averill
 * @brief 
 * @date 15-Sep-2022
 */

#include <stdlib.h>

#include "utils/llvm_stack_entry.h"
#include "utils/logging.h"

/**
 * @brief Initialize a stack entry linked list
 * 
 * @param head Head of linked list
 */
void initialize_stack_entry_linked_list(LLVMStackEntryNode** head)
{
    //*head = NULL;
}

/**
 * @brief Add a stack entry to a stack entry linked list
 * 
 * @param head Head of linked list
 * @param register_index Register number to push onto list
 */
void prepend_stack_entry_linked_list(LLVMStackEntryNode** head, type_register register_index)
{
    LLVMStackEntryNode* temp = (LLVMStackEntryNode*)malloc(sizeof(LLVMStackEntryNode));
    temp->reg = register_index;
    temp->next = NULL;

    if (*head == NULL) {
        *head = temp;
    } else {
        temp->next = *head;
        *head = temp;
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

    type_register out = (*head)->reg;
    temp = (*head)->next;
    free(*head);
    *head = temp;

    return out;
}

/**
 * @brief Free memory used by LLVMStackEntryNode linked lists
 * 
 * @param head Head of list to free
 */
void free_llvm_stack_entry_node_list(LLVMStackEntryNode* head)
{
    LLVMStackEntryNode* current = head;
    LLVMStackEntryNode* prev;
    while (current) {
        prev = current;
        current = current->next;
        free(prev);
    }
    head = NULL;
}