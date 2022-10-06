/**
 * @file symbol_table.c
 * @author Charles Averill
 * @brief Logic for global and local symbol tables
 * @date 16-Sep-2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "translate/symbol_table.h"
#include "utils/hash.h"
#include "utils/logging.h"

/**
 * @brief Create a new Symbol Table Stack
 * 
 * @return SymbolTableStack* Pointer to new empty Symbol Table Stack
 */
SymbolTableStack* new_symbol_table_stack(void)
{
    SymbolTableStack* stack = (SymbolTableStack*)malloc(sizeof(SymbolTableStack));
    stack->length = 0;
    stack->top = NULL;
    return stack;
}

/**
 * @brief Create a new Symbol Table Stack with one empty Symbol Table at the bottom
 * 
 * @return SymbolTableStack* Pointer to new non-empty Symbol Table Stack
 */
SymbolTableStack* new_nonempty_symbol_table_stack(void)
{
    SymbolTableStack* out = new_symbol_table_stack();
    push_symbol_table(out);
    return out;
}

/**
 * @brief Free all memory in a Symbol Table Stack
 */
void free_symbol_table_stack(SymbolTableStack* stack)
{
    while (stack->top) {
        pop_and_free_symbol_table(stack);
    }
}

/**
 * @brief Push new empty Symbol Table onto Symbol Table Stack
 * 
 * @param stack Symbol Table Stack to push new table onto
 */
void push_symbol_table(SymbolTableStack* stack)
{
    push_existing_symbol_table(stack, new_symbol_table());
}

/**
 * @brief Push existing Symbol Table onto Symbol Table Stack
 * 
 * @param stack Symbol Table Stack to push existing table onto
 * @param new_table New table to push onto stack
 */
void push_existing_symbol_table(SymbolTableStack* stack, SymbolTable* new_table)
{
    if (stack->top == NULL) {
        stack->top = new_table;
    } else {
        new_table->next = stack->top;
        stack->top = new_table;
    }
    stack->length++;
}

/**
 * @brief Remove Symbol Table from Symbol Table Stack and return its pointer
 * 
 * @param stack Stack to pop from
 * @return SymbolTable* Pointer to popped Symbol Table
 */
SymbolTable* pop_symbol_table(SymbolTableStack* stack)
{
    SymbolTable* table = stack->top;
    stack->top = stack->top->next;
    stack->length--;
    return table;
}

/**
 * @brief Remove Symbol Table from Symbol Table Stack and free its memory
 * 
 * @param stack Stack to pop from
 */
void pop_and_free_symbol_table(SymbolTableStack* stack)
{
    SymbolTable* table = stack->top;
    stack->top = stack->top->next;
    free(table);
    stack->length--;
}

/**
 * @brief Get pointer of the top Symbol Table from Symbol Table Stack
 * 
 * @param stack Stack to ppek at
 * @return SymbolTable* Pointer to peeked Symbol Table
 */
SymbolTable* peek_symbol_table(SymbolTableStack* stack)
{
    return stack->top;
}

/**
 * @brief Get pointer to empty Symbol Table with length SYMBOL_TABLE_DEFAULT_LENGTH
 * 
 * @return SymbolTable* Pointer to new empty Symbol Table
 */
SymbolTable* new_symbol_table(void)
{
    return new_symbol_table_with_length(SYMBOL_TABLE_DEFAULT_LENGTH);
}

/**
 * @brief Get pointer to empty Symbol Table with a custom length
 * 
 * @return SymbolTable* Pointer to new empty Symbol Table
 */
SymbolTable* new_symbol_table_with_length(int length)
{
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    table->buckets = (SymbolTableEntry**)malloc(sizeof(SymbolTableEntry*) * length);
    table->length = 0;
    table->capacity = length;
    table->total_buckets = length;
    table->next = NULL;
    return table;
}

/**
 * @brief Double the size of a Symbol Table's buckets array
 * 
 * @param table Table to double the size of
 */
void resize_symbol_table(SymbolTable* table)
{
    int new_capacity = table->total_buckets * 2;
    table->buckets =
        (SymbolTableEntry**)realloc(table->buckets, sizeof(SymbolTableEntry*) * new_capacity);
    table->capacity += table->total_buckets;
    table->total_buckets *= 2;
}

/**
 * @brief Find the entry of a symbol in the provided Symbol Table if it exists
 * 
 * @param table Table to search in
 * @param symbol_name Name of symbol to search for
 * @return SymbolTableEntry* Pointer to entry if it exists, else NULL
 */
SymbolTableEntry* find_symbol_table_entry(SymbolTable* table, char* symbol_name)
{
    unsigned long int bucket_index = FNV_1(symbol_name) % table->total_buckets;
    SymbolTableEntry* found = table->buckets[bucket_index];
    while (found != NULL && strcmp(found->symbol_name, symbol_name)) {
        found = found->next;
    }
    return found;
}

/**
 * @brief Get pointer to new Symbol Table Entry
 * 
 * @param symbol_name Name of new symbol
 * @return SymbolTableEntry* Pointer to new Symbol Table Entry
 */
SymbolTableEntry* new_symbol_table_entry(char* symbol_name)
{
    SymbolTableEntry* entry = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
    strcpy(entry->symbol_name, symbol_name);
    entry->length = strlen(symbol_name);
    entry->next = NULL;
    entry->bucket_index = 0;
    entry->chain_index = 0;
    return entry;
}

/**
 * @brief Hash symbol_name with hash function FNV-1 and put it into the chained Symbol Table
 * 
 * @param table Table to put new Symbol Table Entry into
 * @param symbol_name Name of symbol to add
 * @param type Type of symbol to add
 * @return SymbolTableEntry* Pointer to new Symbol Table Entry
 */
SymbolTableEntry* add_symbol_table_entry(SymbolTable* table, char* symbol_name, Type type)
{
    SymbolTableEntry* found = find_symbol_table_entry(table, symbol_name);
    if (found != NULL) {
        identifier_error(0, 0, 0, "Identifier \"%s\" already exists with type \"%s\" in this scope",
                         symbol_name, tokenStrings[found->type.type]);
    }

    SymbolTableEntry* entry = new_symbol_table_entry(symbol_name);
    entry->bucket_index = FNV_1(symbol_name) % table->total_buckets;
    entry->type = type;

    if (table->buckets[entry->bucket_index] != NULL) {
        SymbolTableEntry* curr = table->buckets[entry->bucket_index];
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = entry;
    } else {
        table->buckets[entry->bucket_index] = entry;
    }

    return entry;
}
