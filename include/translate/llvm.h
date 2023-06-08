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
static const char* numberTypeLLVMReprs[] = {"i1", "i8", "i16", "i32", "i64"};

/**
 * @brief Head node of linked list containing register indices that are free to have values stored in them
 */
extern LLVMStackEntryNode* freeVirtualRegistersHead;

#define REFSTRING_BUF_MAXLEN 256
/**
 * @brief Temporary buffer used to generate pointer star strings for LLVM code
 */
static char _refstring_buf[256];

/**
 * @brief Types of values possibly returned by ast_to_llvm
 */
typedef enum
{
    LLVMVALUETYPE_NONE,
    LLVMVALUETYPE_VIRTUAL_REGISTER,
    LLVMVALUETYPE_LABEL,
    LLVMVALUETYPE_CONSTANT,
} LLVMValueType;

static const char* valueTypeStrings[] = {"None", "Virtual Register", "Label", "Constant"};

/**
 * @brief Value returned by ast_to_llvm
 */
typedef struct LLVMValue {
    /**What kind of value is being returned*/
    LLVMValueType value_type;
    /**If a number is stored*/
    NumberType number_type;
    /**How many pointers deep this LLVMValue is*/
    int pointer_depth;
    /**Contents of the value returned*/
    union {
        /**Index of a virtual register*/
        type_register virtual_register_index;
        /**Constant value*/
        long long int constant;
        /**Index of an LLVM label*/
        type_label label_index;
    } value;
} LLVMValue;

#define PRINT_LLVMVALUE(val)                                                                       \
    printf("LLVMValue Information\n");                                                             \
    printf("---------------------\n");                                                             \
    printf("Value Type: %s\n", valueTypeStrings[val.value_type]);                                  \
    printf("Number Type: %s\n", numberTypeLLVMReprs[val.number_type]);                             \
    printf("Pointer Depth: %d\n", val.pointer_depth);                                              \
    printf("Contents: %lld\n", val.value.constant);

/**
 * @brief A standard "null" LLVMValue struct returned in some scenarios
 */
#define LLVMVALUE_NULL                                                                             \
    (LLVMValue)                                                                                    \
    {                                                                                              \
        .value_type = LLVMVALUETYPE_NONE, .value = 0, .pointer_depth = 0                           \
    }

/**
 * @brief Generates an LLVMValue struct with a constant value
 */
#define LLVMVALUE_CONSTANT(c)                                                                      \
    (LLVMValue)                                                                                    \
    {                                                                                              \
        .value_type = LLVMVALUETYPE_CONSTANT, .value.constant = c, .pointer_depth = 0,             \
        .number_type = max_numbertype_for_val(c)                                                   \
    }

/**
 * @brief Inline-initializes an LLVMValue struct from a virtual register number
 */
#define LLVMVALUE_VIRTUAL_REGISTER(register_number, n_t)                                           \
    (LLVMValue)                                                                                    \
    {                                                                                              \
        .value_type = LLVMVALUETYPE_VIRTUAL_REGISTER, .pointer_depth = 0,                          \
        .value.virtual_register_index = register_number, .number_type = n_t                        \
    }

/**
 * @brief Inline-initializes an LLVMValue struct from the number of a virtual register that stores a pointer
 */
#define LLVMVALUE_VIRTUAL_REGISTER_POINTER(register_number, n_t, depth)                            \
    (LLVMValue)                                                                                    \
    {                                                                                              \
        .value_type = LLVMVALUETYPE_VIRTUAL_REGISTER, .pointer_depth = depth,                      \
        .value.virtual_register_index = register_number, .number_type = n_t                        \
    }

/**
 * @brief Inline initializes an LLVMValue struct from a label number
 */
#define LLVMVALUE_LABEL(label_number)                                                              \
    (LLVMValue)                                                                                    \
    {                                                                                              \
        .value_type = LLVMVALUETYPE_LABEL, .value.label_index = label_number, .pointer_depth = 0   \
    }

#define LLVMVALUE_REGMARKER(llvmvalue) (llvmvalue.value_type == LLVMVALUETYPE_CONSTANT ? "" : "%")

/**Prefix to prepend to LLVM label indices*/
#define PURPLE_LABEL_PREFIX "L"

LLVMValue* llvm_ensure_registers_loaded(int n_registers, LLVMValue registers[],
                                        NumberType number_type, int load_depth);

void llvm_preamble(void);
void llvm_postamble(void);

bool llvm_stack_allocation(LLVMStackEntryNode* stack_entries);

LLVMValue llvm_binary_arithmetic(TokenType operation, LLVMValue left_virtual_register,
                                 LLVMValue right_virtual_register);
LLVMValue llvm_store_constant(Number value);
type_register get_next_local_virtual_register(void);
LLVMValue get_next_label(void);

LLVMValue llvm_load_global_variable(char* symbol_name);
void llvm_store_global_variable(char* symbol_name, LLVMValue rvalue_register);
void llvm_declare_global_number_variable(char* symbol_name, Number n);
LLVMValue llvm_int_resize(LLVMValue reg, NumberType new_tye);
void llvm_declare_assign_global_number_variable(char* symbol_name, Number number);
void llvm_print_int(LLVMValue print_vr);
void llvm_print_bool(LLVMValue print_vr);
LLVMValue llvm_compare(TokenType comparison_type, LLVMValue left_virtual_register,
                       LLVMValue right_virtual_register);
LLVMValue llvm_compare_jump(TokenType comparison_type, LLVMValue left_virtual_register,
                            LLVMValue right_virtual_register, LLVMValue false_label);
void llvm_label(LLVMValue label);
void llvm_jump(LLVMValue label);
void llvm_conditional_jump(LLVMValue condition_register, LLVMValue true_label,
                           LLVMValue false_label);
void llvm_function_preamble(char* symbol_name);
void llvm_function_postamble(void);
LLVMValue llvm_call_function(LLVMValue virtual_register, char* symbol_name);
const char* type_to_llvm_type(TokenType type);
void llvm_return(LLVMValue virtual_register, char* symbol_name);
char* refstring(char* buf, int pointer_depth);
LLVMValue llvm_get_address(char* symbol_name);
LLVMValue llvm_dereference(LLVMValue reg);

/**
 * @brief Wrapper for _refstring - WARNING - only one call to REFSTRING may be made per statement, due to 
 * _refstring_buf being used in it. Multiple uses will overwrite all but the last occurrance
 */
#define REFSTRING(depth) refstring(_refstring_buf, depth)

#endif /* LLVM_H */
