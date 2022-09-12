/**
 * @file translate.h
 * @author Charles Averill
 * @brief Function headers and definitions for translation of an AST into LLVM-IR
 * @date 10-Sep-2022
 */

#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <stdio.h>
#include <stdlib.h>

#include "parse.h"
#include "tree.h"

/**
 * @brief Types of values possibly returned by ast_to_llvm
 */
typedef enum {
    LLVMVALUETYPE_NONE,
    LLVMVALUETYPE_VIRTUAL_REGISTER,
} LLVMValueType;

/**
 * @brief Value returned by ast_to_llvm
 */
typedef struct LLVMValue {
    /**What kind of value is being returned*/
    LLVMValueType value_type;
    /**Contents of the value returned*/
    union {
        int virtual_register_index;
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
        .value_type = LLVMVALUETYPE_VIRTUAL_REGISTER,                                              \
        .value.virtual_register_index = register_number                                            \
    }

LLVMValue ast_to_llvm(ASTNode* n);
void generate_llvm(ASTNode* root);

#endif /* TRANSLATE_H */
