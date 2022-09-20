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
#include "utils/llvm_stack_entry.h"

/**
 * @brief LLVM-IR representations of data types
 */
static const char* numberTypeLLVMReprs[] = {
    "i32",
};

/**
 * @brief Head node of linked list containing register indices that have loaded values
 */
extern LLVMStackEntryNode* loadedRegistersHead;

/**
 * @brief Head node of linked list containing register indices that are free to have values stored in them
 */
extern LLVMStackEntryNode* freeVirtualRegistersHead;

/**
 * @brief Types of values possibly returned by ast_to_llvm
 */
typedef enum
{
    LLVMVALUETYPE_NONE,
    LLVMVALUETYPE_VIRTUAL_REGISTER,
} LLVMValueType;

/**
 * @brief Value returned by ast_to_llvm
 */
typedef struct LLVMValue {
    /**What kind of value is being returned*/
    LLVMValueType value_type;
    /**Stores a pointer?*/
    bool stores_pointer;
    /**Contents of the value returned*/
    union {
        type_register virtual_register_index;
    } value;
} LLVMValue;

/**
 * @brief A standard "null" LLVMValue struct returned in some scenarios
 */
#define LLVMVALUE_NULL                                                                             \
    (LLVMValue) { .value_type = LLVMVALUETYPE_NONE, .value = 0 }

/**
 * @brief Inline-initializes an LLVMValue struct from a virtual register number
 */
#define LLVMVALUE_VIRTUAL_REGISTER(register_number)                                                \
    (LLVMValue)                                                                                    \
    {                                                                                              \
        .value_type = LLVMVALUETYPE_VIRTUAL_REGISTER, .stores_pointer = false,                     \
        .value.virtual_register_index = register_number                                            \
    }

/**
 * @brief Inline-initializes an LLVMValue struct from the number of a virtual register that stores a pointer
 */
#define LLVMVALUE_VIRTUAL_REGISTER_POINTER(register_number)                                        \
    (LLVMValue)                                                                                    \
    {                                                                                              \
        .value_type = LLVMVALUETYPE_VIRTUAL_REGISTER, .stores_pointer = true,                      \
        .value.virtual_register_index = register_number                                            \
    }

type_register* llvm_ensure_registers_loaded(int n_registers, type_register registers[]);

void llvm_preamble();
void llvm_postamble();

void llvm_stack_allocation(LLVMStackEntryNode* stack_entries);

LLVMValue llvm_binary_arithmetic(TokenType operation, LLVMValue left_virtual_register,
                                 LLVMValue right_virtual_register);
LLVMValue llvm_store_constant(Number value);
type_register get_next_local_virtual_register(void);

LLVMValue llvm_load_global_variable(char* symbol_name);
void llvm_store_global_variable(char* symbol_name, type_register rvalue_register_number);
void llvm_declare_global_number_variable(char* symbol_name, NumberType number_type);
void llvm_declare_assign_global_number_variable(char* symbol_name, Number number);
void llvm_print_int(type_register reg);

#endif /* LLVM_H */
