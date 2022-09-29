/**
 * @file symbol_table.h
 * @author Charles Averill
 * @brief Function headers and definitions for the global and local symbol tables
 * @date 16-Sep-2022
 */

#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE

#include <stdbool.h>

#include "types/type.h"

#define SYMBOL_TABLE_DEFAULT_LENGTH 1024

/**Purple identifiers can be a maximum of 255 bytes in length*/
#define D_MAX_IDENTIFIER_LENGTH 255

/**
 * @brief Struct holding data about a symbol
 */
typedef struct SymbolTableEntry {
    /**Name of symbol*/
    char symbol_name[D_MAX_IDENTIFIER_LENGTH + 1];
    /**Length of name*/
    unsigned int length;
    /**Index of symbol in Symbol Table*/
    unsigned long int bucket_index;
    /**Type of entry*/
    Type type;
    /**Symbol Tables are a chained Hash Table, this is the chain*/
    struct SymbolTableEntry* next;
    /**Index in chain*/
    unsigned int chain_index;
} SymbolTableEntry;

/**
 * @brief Holds data for symbols within a scope
 */
typedef struct SymbolTable {
    /**Number of non-empty buckets in the Symbol Table*/
    unsigned long int length;
    /**Number of empty buckets in the Symbol Table*/
    unsigned long int capacity;
    /**Total number of buckets in the Symbol Table*/
    unsigned long int total_buckets;
    /**Array of entries with length length*/
    SymbolTableEntry** buckets;
    /**Next Symbol Table in the scope stack*/
    struct SymbolTable* next;
} SymbolTable;

/**
 * @brief Stack of Symbol Tables used for scoping
 */
typedef struct SymbolTableStack {
    unsigned long long int length;
    SymbolTable* top;
} SymbolTableStack;

// Symbol Table Stack functions
SymbolTableStack* new_symbol_table_stack(void);
SymbolTableStack* new_nonempty_symbol_table_stack(void);
void free_symbol_table_stack(SymbolTableStack* stack);
void push_symbol_table(SymbolTableStack* stack);
void push_existing_symbol_table(SymbolTableStack* stack, SymbolTable* new_table);

// Symbol Table functions
SymbolTable* pop_symbol_table(SymbolTableStack* stack);
void pop_and_free_symbol_table(SymbolTableStack* stack);
SymbolTable* peek_symbol_table(SymbolTableStack* stack);
SymbolTable* new_symbol_table(void);
SymbolTable* new_symbol_table_with_length(int length);
void resize_symbol_table(SymbolTable* table);
SymbolTableEntry* find_symbol_table_entry(SymbolTable* table, char* symbol_name);

// Symbol Table Entry functions
SymbolTableEntry* new_symbol_table_entry(char* symbol_name);
SymbolTableEntry* add_symbol_table_entry(SymbolTable* table, char* symbol_name, Type type);

#endif /* SYMBOL_TABLE */
