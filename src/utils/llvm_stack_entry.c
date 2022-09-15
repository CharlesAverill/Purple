/**
 * @file llvm_stack_entry.c
 * @author Charles Averill
 * @brief 
 * @date 15-Sep-2022
 */

#include "utils/llvm_stack_entry.h"
#include "utils/logging.h"

void initialize_stack_entry_linked_list(LLVMStackEntryNode** head) { *head = NULL; }

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