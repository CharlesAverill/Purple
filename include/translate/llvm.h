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
 * @brief A too-big data type for register indices in case of stress testing
 */
#define type_register unsigned long long int

/**
 * @brief Node of a linked list containing information about required stack allocation for a function. Also used as a general-purpose linked list of registers with loaded values
 */
typedef struct LLVMStackEntryNode {
    /**Register number of the current node*/
    type_register reg;
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

void llvm_print_int(type_register reg);

#endif /* LLVM_H */
