/**
 * @file llvm.c
 * @author Charles Averill
 * @brief LLVM-IR emission
 * @date 12-Sep-2022
 */

#include <string.h>

#include "data.h"
#include "translate/llvm.h"
#include "translate/translate.h"
#include "types/type.h"
#include "utils/clang.h"
#include "utils/formatting.h"
#include "utils/logging.h"

/**
 * @brief Update loaded register linked list to include new register
 * 
 * @param reg Register to include in list
 */
void prepend_loaded(type_register reg)
{
    LLVMStackEntryNode* new = (LLVMStackEntryNode*)malloc(sizeof(LLVMStackEntryNode));
    new->reg = reg;
    new->next = NULL;

    if (loadedRegistersHead == NULL) {
        loadedRegistersHead = new;
    } else {
        LLVMStackEntryNode* temp = loadedRegistersHead;
        while (temp != NULL) {
            if (temp == new) {
                return;
            }
            temp = temp->next;
        }
        new->next = loadedRegistersHead;
        loadedRegistersHead = new;
    }
}

/**
 * @brief Ensure that the values of a set of registers are loaded
 * 
 * @param n_registers Number of registers to ensure
 * @param registers Array of register indices to ensure
 * @param number_type NumberType of registers to ensure
 * @return type_register* If the registers were not loaded, this array contains the loaded registers
 */
type_register* llvm_ensure_registers_loaded(int n_registers, type_register registers[],
                                            NumberType number_type)
{
    LLVMStackEntryNode* current = loadedRegistersHead;
    bool found_registers[n_registers];

    for (int i = 0; i < n_registers; i++) {
        found_registers[i] = 0;
    }

    int found = 0;
    while (current) {
        for (int i = 0; i < n_registers; i++) {
            if (!found_registers[i] && current->reg == registers[i]) {
                found_registers[i] = true;
                found++;
            }

            if (found == n_registers) {
                return NULL;
            }
        }
        current = current->next;
    }

    // Haven't loaded all of our registers yet
    type_register* loaded_registers =
        (type_register*)malloc(sizeof(type_register) * (n_registers - found));
    for (int i = 0; i < n_registers; i++) {
        loaded_registers[i] = registers[i];
        if (!found_registers[i]) {
            loaded_registers[i] = get_next_local_virtual_register();
            fprintf(D_LLVM_FILE, TAB "%%%llu = load %s, %s* %%%llu, align %d" NEWLINE,
                    loaded_registers[i], numberTypeLLVMReprs[number_type],
                    numberTypeLLVMReprs[number_type], registers[i], numberTypeByteSizes[i]);
            prepend_loaded(loaded_registers[i]);
        }
    }

    return loaded_registers;
}

/**
 * @brief Generated program's preamble
 */
void llvm_preamble(void)
{
    fprintf(D_LLVM_FILE, "; ModuleID = '%s'" NEWLINE, D_INPUT_FN);

    // Target layout
    char* target_datalayout = get_target_datalayout();
    fprintf(D_LLVM_FILE, "target datalayout = \"%s\"" NEWLINE, target_datalayout);
    purple_log(LOG_DEBUG, "Freeing %s", "target_datalayout");
    free(target_datalayout);

    // Target triple
    char* target_triple = get_target_triple();
    fprintf(D_LLVM_FILE, "target triple = \"%s\"" NEWLINE NEWLINE, target_triple);
    purple_log(LOG_DEBUG, "Freeing %s", "target_triple");
    free(target_triple);

    // Globals placeholder
    fprintf(D_LLVM_FILE, PURPLE_GLOBALS_PLACEHOLDER NEWLINE NEWLINE);

    fprintf(D_LLVM_FILE, "@print_int_fstring = private unnamed_addr constant [4 x i8] "
                         "c\"%%d\\0A\\00\", align 1" NEWLINE NEWLINE);
    fprintf(D_LLVM_FILE, "@print_long_fstring = private unnamed_addr constant [5 x i8] "
                         "c\"%%ld\\0A\\00\", align 1" NEWLINE NEWLINE);
    fprintf(D_LLVM_FILE, "@print_char_fstring = private unnamed_addr constant [4 x i8] "
                         "c\"%%c\\0A\\00\", align 1" NEWLINE NEWLINE);
    fprintf(D_LLVM_FILE, "@print_true_fstring = private unnamed_addr constant [6 x i8] "
                         "c\"true\\0A\\00\", align 1" NEWLINE NEWLINE);
    fprintf(D_LLVM_FILE, "@print_false_fstring = private unnamed_addr constant [7 x i8] "
                         "c\"false\\0A\\00\", align 1" NEWLINE NEWLINE);
    fprintf(D_LLVM_FILE, "; Function Attrs: noinline nounwind optnone uwtable" NEWLINE);
}

/**
 * @brief Generated program's postamble
 */
void llvm_postamble(void)
{
    fprintf(D_LLVM_FILE, "declare i32 @printf(i8*, ...) #1" NEWLINE NEWLINE);
    fprintf(
        D_LLVM_FILE,
        "attributes #0 = { noinline nounwind optnone uwtable \"frame-pointer\"=\"all\" "
        "\"min-legal-vector-width\"=\"0\" \"no-trapping-math\"=\"true\" "
        "\"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" "
        "\"target-features\"=\"+cx8,+fxsr,+mmx,+sse,+sse2,+x87\" \"tune-cpu\"=\"generic\" }" NEWLINE
            NEWLINE);
    fprintf(
        D_LLVM_FILE,
        "attributes #1 = { \"frame-pointer\"=\"all\" \"no-trapping-math\"=\"true\" "
        "\"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" "
        "\"target-features\"=\"+cx8,+fxsr,+mmx,+sse,+sse2,+x87\" \"tune-cpu\"=\"generic\" }" NEWLINE
            NEWLINE);
    fprintf(D_LLVM_FILE, "!llvm.module.flags = !{!0, !1, !2, !3, !4}" NEWLINE);
    fprintf(D_LLVM_FILE, "!llvm.ident = !{!5}" NEWLINE NEWLINE);
    fprintf(D_LLVM_FILE, "!0 = !{i32 1, !\"wchar_size\", i32 4}" NEWLINE);
    fprintf(D_LLVM_FILE, "!1 = !{i32 7, !\"PIC Level\", i32 2}" NEWLINE);
    fprintf(D_LLVM_FILE, "!2 = !{i32 7, !\"PIE Level\", i32 2}" NEWLINE);
    fprintf(D_LLVM_FILE, "!3 = !{i32 7, !\"uwtable\", i32 1}" NEWLINE);
    fprintf(D_LLVM_FILE, "!4 = !{i32 7, !\"frame-pointer\", i32 2}" NEWLINE);
    fprintf(D_LLVM_FILE, "!5 = !{!\"Ubuntu clang version 14.0.0-1ubuntu1\"}" NEWLINE);
}

LLVMStackEntryNode* buffered_stack_entries_head = NULL;

/**
 * @brief Allocate space on stack for variables
 * 
 * @param stack_entries LLVMStackEntryNode pointers holding stack allocation information
 * @return bool True if stack_entries may be freed
 */
bool llvm_stack_allocation(LLVMStackEntryNode* stack_entries)
{
    LLVMStackEntryNode* current = stack_entries;

    // If the preamble hasn't been printed, buffer the stack entries and allocate them once it is printed
    if (!D_CURRENT_FUNCTION_PREAMBLE_PRINTED) {
        if (buffered_stack_entries_head == NULL) {
            buffered_stack_entries_head = stack_entries;
        } else {
            LLVMStackEntryNode* current_buffer = buffered_stack_entries_head;
            while (current_buffer->next != NULL) {
                current_buffer = current_buffer->next;
            }
            current_buffer->next = stack_entries;
        }

        return false;
    }

    while (current) {
        fprintf(D_LLVM_FILE, TAB "%%%llu = alloca %s, align %d" NEWLINE, current->reg,
                numberTypeLLVMReprs[current->type], current->align_bytes);
        current = current->next;
    }

    return true;
}

/**
 * @brief Generate code for binary addition
 * 
 * @param left_virtual_register Lvalue to be added
 * @param right_virtual_register Rvalue to be added
 * @return type_register Virtual register holding result
 */
static type_register llvm_add(LLVMValue left_virtual_register, LLVMValue right_virtual_register)
{
    fprintf(D_LLVM_FILE, TAB "%%%llu = add nsw %s %%%llu, %%%llu" NEWLINE,
            get_next_local_virtual_register(),
            numberTypeLLVMReprs[left_virtual_register.number_type],
            left_virtual_register.value.virtual_register_index,
            right_virtual_register.value.virtual_register_index);
    return D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER - 1;
}

/**
 * @brief Generate code for binary subtraction
 * 
 * @param left_virtual_register Lvalue to be subtracted
 * @param right_virtual_register Rvalue to be subtracted
 * @return type_register Virtual register holding result
 */
static type_register llvm_subtract(LLVMValue left_virtual_register,
                                   LLVMValue right_virtual_register)
{
    fprintf(D_LLVM_FILE, TAB "%%%llu = sub nsw %s %%%llu, %%%llu" NEWLINE,
            get_next_local_virtual_register(),
            numberTypeLLVMReprs[left_virtual_register.number_type],
            left_virtual_register.value.virtual_register_index,
            right_virtual_register.value.virtual_register_index);
    return D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER - 1;
}

/**
 * @brief Generate code for binary multiplication
 * 
 * @param left_virtual_register Lvalue to be multiplied
 * @param right_virtual_register Rvalue to be multiplied
 * @return type_register Virtual register holding result
 */
static type_register llvm_multiply(LLVMValue left_virtual_register,
                                   LLVMValue right_virtual_register)
{
    fprintf(D_LLVM_FILE, TAB "%%%llu = mul nsw %s %%%llu, %%%llu" NEWLINE,
            get_next_local_virtual_register(),
            numberTypeLLVMReprs[left_virtual_register.number_type],
            left_virtual_register.value.virtual_register_index,
            right_virtual_register.value.virtual_register_index);
    return D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER - 1;
}

/**
 * @brief Generate code for unsigned binary division
 * 
 * @param left_virtual_register Lvalue to be divided
 * @param right_virtual_register Rvalue to be divided
 * @return type_register Virtual register holding result
 */
static type_register llvm_divide(LLVMValue left_virtual_register, LLVMValue right_virtual_register)
{
    fprintf(D_LLVM_FILE, TAB "%%%llu = udiv %s %%%llu, %%%llu" NEWLINE,
            get_next_local_virtual_register(),
            numberTypeLLVMReprs[left_virtual_register.number_type],
            left_virtual_register.value.virtual_register_index,
            right_virtual_register.value.virtual_register_index);
    return D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER - 1;
}

/**
 * @brief Generates LLVM-IR for various binary arithmetic expressions
 * 
 * @param operation Operation to perform
 * @param left_virtual_register Operand left of operation
 * @param right_virtual_register Operand right of operation
 * @return int Number of virtual register in which the result is stored
 */
LLVMValue llvm_binary_arithmetic(TokenType operation, LLVMValue left_virtual_register,
                                 LLVMValue right_virtual_register)
{
    type_register out_register;

    type_register* loaded_registers = llvm_ensure_registers_loaded(
        1, (type_register[]){left_virtual_register.value.virtual_register_index},
        left_virtual_register.number_type);
    if (loaded_registers != NULL) {
        left_virtual_register.value.virtual_register_index = loaded_registers[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers (1)", "llvm_binary_arithmetic");
        free(loaded_registers);
        loaded_registers = NULL;
    }

    loaded_registers = llvm_ensure_registers_loaded(
        1, (type_register[]){right_virtual_register.value.virtual_register_index},
        right_virtual_register.number_type);
    if (loaded_registers != NULL) {
        right_virtual_register.value.virtual_register_index = loaded_registers[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers (2)", "llvm_binary_arithmetic");
        free(loaded_registers);
    }

    if (left_virtual_register.number_type != right_virtual_register.number_type) {
        if (numberTypeBitSizes[left_virtual_register.number_type] <
            numberTypeBitSizes[right_virtual_register.number_type]) {
            left_virtual_register =
                llvm_signed_extend(left_virtual_register, right_virtual_register.number_type,
                                   left_virtual_register.number_type);
            prepend_loaded(left_virtual_register.value.virtual_register_index);
        } else {
            right_virtual_register =
                llvm_signed_extend(right_virtual_register, left_virtual_register.number_type,
                                   right_virtual_register.number_type);

            prepend_loaded(right_virtual_register.value.virtual_register_index);
        }
    }

    switch (operation) {
    case T_PLUS:
        out_register = llvm_add(left_virtual_register, right_virtual_register);
        break;
    case T_MINUS:
        out_register = llvm_subtract(left_virtual_register, right_virtual_register);
        break;
    case T_STAR:
        out_register = llvm_multiply(left_virtual_register, right_virtual_register);
        break;
    case T_SLASH:
        out_register = llvm_divide(left_virtual_register, right_virtual_register);
        break;
    case T_EXPONENT:
        fatal(RC_COMPILER_ERROR,
              "Exponent not yet supported, as libc pow only takes floating-point types");
        break;
    default:
        fatal(RC_COMPILER_ERROR,
              "llvm_binary_arithmetic receieved non-binary-arithmetic operator \"%s\"",
              tokenStrings[operation]);
    }

    prepend_loaded(out_register);

    return LLVMVALUE_VIRTUAL_REGISTER(out_register, left_virtual_register.number_type);
}

/**
 * @brief Store a constant number value into a register
 * 
 * @param value Number struct containing information about the constant
 * @return int Register number value is held in
 */
LLVMValue llvm_store_constant(Number value)
{
    purple_log(LOG_DEBUG, "Storing constant value %ld", value.value);
    type_register out_register_number = pop_stack_entry_linked_list(&freeVirtualRegistersHead);
    fprintf(D_LLVM_FILE, TAB "store %s ", numberTypeLLVMReprs[value.type]);
    fprintf(D_LLVM_FILE, numberTypeFormatStrings[value.type], value.value);
    fprintf(D_LLVM_FILE, ", %s* %%%llu, align %d" NEWLINE, numberTypeLLVMReprs[value.type],
            out_register_number, numberTypeByteSizes[value.type]);
    return LLVMVALUE_VIRTUAL_REGISTER_POINTER(out_register_number, value.type);
}

/**
 * @brief Retrieves the next valid virtual register index
 * 
 * @return type_register Index of next unused virtual register
 */
type_register get_next_local_virtual_register(void)
{
    return D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER++;
}

/**
 * @brief Load a global variable's value into a new virtual register
 * 
 * @param symbol_name Identifier name of variable to load
 * @return LLVMValue Register number variable value is held in
 */
LLVMValue llvm_load_global_variable(char* symbol_name)
{
    type_register out_register_number = get_next_local_virtual_register();

    SymbolTableEntry* symbol = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, symbol_name);
    if (symbol == NULL) {
        fatal(RC_COMPILER_ERROR, "Failed to find symbol \"%s\" in Global Symbol Table",
              symbol_name);
    }

    fprintf(D_LLVM_FILE, TAB "%%%llu = load %s, %s* @%s" NEWLINE, out_register_number,
            numberTypeLLVMReprs[symbol->type.value.number.type],
            numberTypeLLVMReprs[symbol->type.value.number.type], symbol_name);
    prepend_loaded(out_register_number);
    return LLVMVALUE_VIRTUAL_REGISTER(out_register_number, symbol->type.value.number.type);
}

/**
 * @brief Store a value into a global variable
 * 
 * @param symbol_name Identifier name of variable to store new value to
 * @param rvalue_register Register number of statement's RValue to store
 */
void llvm_store_global_variable(char* symbol_name, LLVMValue rvalue_register)
{
    SymbolTableEntry* symbol = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, symbol_name);
    if (symbol == NULL) {
        fatal(RC_COMPILER_ERROR, "Failed to find symbol \"%s\" in Global Symbol Table",
              symbol_name);
    }

    type_register* loaded_registers = llvm_ensure_registers_loaded(
        1, (type_register[]){rvalue_register.value.virtual_register_index},
        rvalue_register.number_type);
    if (loaded_registers) {
        rvalue_register.value.virtual_register_index = loaded_registers[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers", "llvm_store_global_variable");
        free(loaded_registers);
    }

    if (TOKENTYPE_IS_NUMBER_TYPE(symbol->type.type) &&
        rvalue_register.number_type != symbol->type.value.number.type) {
        if (numberTypeBitSizes[rvalue_register.number_type] <
            numberTypeBitSizes[symbol->type.value.number.type]) {
            rvalue_register = llvm_signed_extend(rvalue_register, symbol->type.value.number.type,
                                                 rvalue_register.number_type);
        } else {
            rvalue_register = llvm_truncate(rvalue_register, symbol->type.value.number.type,
                                            rvalue_register.number_type);
        }

        prepend_loaded(rvalue_register.value.virtual_register_index);
    }

    fprintf(D_LLVM_FILE, TAB "store %s %%%llu, %s* @%s" NEWLINE,
            numberTypeLLVMReprs[symbol->type.value.number.type],
            rvalue_register.value.virtual_register_index,
            numberTypeLLVMReprs[symbol->type.value.number.type], symbol_name);
}

LLVMValue llvm_signed_extend(LLVMValue reg, NumberType new_type, NumberType old_type)
{
    if (new_type == old_type) {
        return LLVMVALUE_NULL;
    }

    LLVMValue out = LLVMVALUE_VIRTUAL_REGISTER(get_next_local_virtual_register(), new_type);
    fprintf(D_LLVM_FILE, TAB "%%%llu = sext %s %%%llu to %s" NEWLINE,
            out.value.virtual_register_index, numberTypeLLVMReprs[old_type],
            reg.value.virtual_register_index, numberTypeLLVMReprs[new_type]);
    return out;
}

LLVMValue llvm_truncate(LLVMValue reg, NumberType new_type, NumberType old_type)
{
    if (new_type == old_type) {
        return LLVMVALUE_NULL;
    }

    LLVMValue out = LLVMVALUE_VIRTUAL_REGISTER(get_next_local_virtual_register(), new_type);
    fprintf(D_LLVM_FILE, TAB "%%%llu = trunc %s %%%llu to %s" NEWLINE,
            out.value.virtual_register_index, numberTypeLLVMReprs[old_type],
            reg.value.virtual_register_index, numberTypeLLVMReprs[new_type]);
    return out;
}

/**
 * @brief Declare a global variable
 * 
 * @param symbol_name Name of global variable
 * @param number_type Type of number of global variable
 */
void llvm_declare_global_number_variable(char* symbol_name, NumberType number_type)
{
    fprintf(D_LLVM_GLOBALS_FILE, "@%s = global %s 0" NEWLINE, symbol_name,
            numberTypeLLVMReprs[number_type]);
}

/**
 * @brief Declare a global variable with an assigned number value
 * 
 * @param symbol_name Name of global variable
 * @param number Default value of global variable
 */
void llvm_declare_assign_global_number_variable(char* symbol_name, Number number)
{
    fprintf(D_LLVM_GLOBALS_FILE, "@%s = global %s %lld" NEWLINE, symbol_name,
            numberTypeLLVMReprs[number.type], number.value);
}

/**
 * @brief Generate code to print an integer
 * 
 * @param print_vr Register holding value to print
 * @param type Type of int (byte, char, int, long) to print
 */
void llvm_print_int(type_register print_vr, TokenType type)
{
    type_register* loaded_register = llvm_ensure_registers_loaded(1, (type_register[]){print_vr},
                                                                  token_type_to_number_type(type));
    if (loaded_register) {
        print_vr = loaded_register[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_register", "llvm_print_int");
        free(loaded_register);
    }

    get_next_local_virtual_register();
    switch (type) {
    case T_CHAR:
    case T_CHAR_LITERAL:
        fprintf(D_LLVM_FILE,
                TAB "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x "
                    "i8]* @print_char_fstring , i32 0, i32 0), %s %%%llu)" NEWLINE,
                numberTypeLLVMReprs[NT_INT8], print_vr);
        break;
    case T_BYTE:
    case T_BYTE_LITERAL:
    case T_SHORT:
    case T_SHORT_LITERAL:
    case T_INT:
    case T_INTEGER_LITERAL:
        fprintf(D_LLVM_FILE,
                TAB "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x "
                    "i8]* @print_int_fstring , i32 0, i32 0), %s %%%llu)" NEWLINE,
                numberTypeLLVMReprs[token_type_to_number_type(type)], print_vr);
        break;
    case T_LONG:
    case T_LONG_LITERAL:
        fprintf(D_LLVM_FILE,
                TAB "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x "
                    "i8]* @print_long_fstring , i32 0, i32 0), %s %%%llu)" NEWLINE,
                numberTypeLLVMReprs[NT_INT64], print_vr);
        break;
    default:
        fatal(RC_COMPILER_ERROR, "Unrecognized TokenType %s", tokenStrings[type]);
    }
}

/**
 * @brief Generate code to print a boolean value
 * 
 * @param print_vr Register holding value to print
 */
void llvm_print_bool(type_register print_vr)
{
    type_register* loaded_register =
        llvm_ensure_registers_loaded(1, (type_register[]){print_vr}, NT_INT1);
    if (loaded_register) {
        print_vr = loaded_register[0];
    }

    type_register compare_register = print_vr;
    LLVMValue true_label, false_label, end_label;
    true_label = get_next_label();
    false_label = get_next_label();
    end_label = get_next_label();

    llvm_conditional_jump(LLVMVALUE_VIRTUAL_REGISTER(compare_register, NT_INT1), true_label,
                          false_label);
    llvm_label(true_label);
    get_next_local_virtual_register();
    fprintf(D_LLVM_FILE,
            TAB "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x "
                "i8]* @print_true_fstring , i32 0, i32 0))" NEWLINE);
    llvm_jump(end_label);
    llvm_label(false_label);
    get_next_local_virtual_register();
    fprintf(D_LLVM_FILE,
            TAB "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x "
                "i8]* @print_false_fstring , i32 0, i32 0))" NEWLINE);
    llvm_jump(end_label);
    llvm_label(end_label);
}

static void llvm_relational_compare(TokenType comparison_type, type_register out_register,
                                    LLVMValue left_virtual_register,
                                    LLVMValue right_virtual_register)
{
    fprintf(D_LLVM_FILE, TAB "%%%llu = icmp ", out_register);

    switch (comparison_type) {
    case T_EQ:
        fprintf(D_LLVM_FILE, "eq");
        break;
    case T_NEQ:
        fprintf(D_LLVM_FILE, "ne");
        break;
    case T_LT:
        fprintf(D_LLVM_FILE, "slt");
        break;
    case T_LE:
        fprintf(D_LLVM_FILE, "sle");
        break;
    case T_GT:
        fprintf(D_LLVM_FILE, "sgt");
        break;
    case T_GE:
        fprintf(D_LLVM_FILE, "sge");
        break;
    default:
        fatal(RC_COMPILER_ERROR,
              "llvm_relational_compare receieved non-relational-comparison operator \"%s\"",
              tokenStrings[comparison_type]);
    }

    fprintf(D_LLVM_FILE, " %s %%%llu, %%%llu" NEWLINE,
            numberTypeLLVMReprs[left_virtual_register.number_type],
            left_virtual_register.value.virtual_register_index,
            right_virtual_register.value.virtual_register_index);
}

static void llvm_logical_compare(TokenType comparison_type, type_register out_register,
                                 LLVMValue left_virtual_register, LLVMValue right_virtual_register)
{
    fprintf(D_LLVM_FILE, TAB "%%%llu = ", out_register);

    switch (comparison_type) {
    case T_AND:
        fprintf(D_LLVM_FILE, "and");
        break;
    case T_OR:
        fprintf(D_LLVM_FILE, "or");
        break;
    case T_XOR:
        fprintf(D_LLVM_FILE, "xor");
        break;
    case T_NAND:
    case T_NOR:
    case T_XNOR:
        fatal(RC_COMPILER_ERROR, "N- logical operators not yet supported");
    default:
        fatal(RC_COMPILER_ERROR, "llvm_logical_compare received Token \"%s\"",
              tokenStrings[comparison_type]);
    }

    fprintf(D_LLVM_FILE, " %s %%%llu, %%%llu" NEWLINE,
            numberTypeLLVMReprs[left_virtual_register.number_type],
            left_virtual_register.value.virtual_register_index,
            right_virtual_register.value.virtual_register_index);
}

/**
 * @brief Generate code to compare two registers
 * 
 * @param comparison_type Type of comparison to make
 * @param left_virtual_register LLVMValue storing left value register index
 * @param right_virtual_register LLVMValue storing right value register index
 * @return LLVMValue Register index of comparison value
 */
LLVMValue llvm_compare(TokenType comparison_type, LLVMValue left_virtual_register,
                       LLVMValue right_virtual_register)
{
    type_register* loaded_registers = llvm_ensure_registers_loaded(
        1, (type_register[]){left_virtual_register.value.virtual_register_index},
        left_virtual_register.number_type);
    if (loaded_registers != NULL) {
        left_virtual_register.value.virtual_register_index = loaded_registers[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers", "llvm_compare");
        free(loaded_registers);
    }

    loaded_registers = llvm_ensure_registers_loaded(
        1, (type_register[]){right_virtual_register.value.virtual_register_index},
        right_virtual_register.number_type);
    if (loaded_registers != NULL) {
        right_virtual_register.value.virtual_register_index = loaded_registers[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers", "llvm_compare");
        free(loaded_registers);
        loaded_registers = NULL;
    }

    if (left_virtual_register.number_type != right_virtual_register.number_type) {
        if (numberTypeBitSizes[left_virtual_register.number_type] <
            numberTypeBitSizes[right_virtual_register.number_type]) {
            left_virtual_register =
                llvm_signed_extend(left_virtual_register, right_virtual_register.number_type,
                                   left_virtual_register.number_type);
            prepend_loaded(left_virtual_register.value.virtual_register_index);
        } else {
            right_virtual_register =
                llvm_signed_extend(right_virtual_register, left_virtual_register.number_type,
                                   right_virtual_register.number_type);

            prepend_loaded(right_virtual_register.value.virtual_register_index);
        }
    }

    type_register out_register = get_next_local_virtual_register();

    if (TOKENTYPE_IS_COMPARATOR(comparison_type)) {
        llvm_relational_compare(comparison_type, out_register, left_virtual_register,
                                right_virtual_register);
    } else {
        llvm_logical_compare(comparison_type, out_register, left_virtual_register,
                             right_virtual_register);
    }

    prepend_loaded(out_register);

    return LLVMVALUE_VIRTUAL_REGISTER(out_register, NT_INT1);
}

/**
 * @brief Generate code to compare two registers and conditionally jump based on the result
 * 
 * @param comparison_type Type of comparison to make
 * @param left_virtual_register LLVMValue storing left value register index
 * @param right_virtual_register LLVMValue storing right value register index
 * @param false_label LLVMValue storing label data for the branch in which the condition is false
 * @return LLVMValue Register index of comparison value
 */
LLVMValue llvm_compare_jump(TokenType comparison_type, LLVMValue left_virtual_register,
                            LLVMValue right_virtual_register, LLVMValue false_label)
{
    LLVMValue comparison_result =
        llvm_compare(comparison_type, left_virtual_register, right_virtual_register);

    if (comparison_result.value_type != LLVMVALUETYPE_VIRTUAL_REGISTER) {
        fatal(RC_COMPILER_ERROR, "Tried to generate jump after comparison, but comparison did not "
                                 "return a virtual register number");
    }

    LLVMValue true_label = get_next_label();

    llvm_conditional_jump(comparison_result, true_label, false_label);

    llvm_label(true_label);

    return comparison_result;
}

/**
 * @brief Get the next valid label
 * 
 * @return LLVMValue Next valid label
 */
LLVMValue get_next_label(void)
{
    return LLVMVALUE_LABEL(D_LABEL_INDEX++);
}

/**
 * @brief Generate label code
 * 
 * @param label LLVMValue containing label information
 */
void llvm_label(LLVMValue label)
{
    if (label.value_type != LLVMVALUETYPE_LABEL) {
        fatal(RC_COMPILER_ERROR,
              "Tried to generate a label statement, but received a non-label LLVMValue");
    }

    fprintf(D_LLVM_FILE, TAB PURPLE_LABEL_PREFIX "%llu:" NEWLINE, label.value.label_index);
}

/**
 * @brief Generate an unconditional jump statement
 * 
 * @param label Label to jump to
 */
void llvm_jump(LLVMValue label)
{
    if (label.value_type != LLVMVALUETYPE_LABEL) {
        fatal(RC_COMPILER_ERROR,
              "Tried to generate a label statement, but received a non-label LLVMValue");
    }

    fprintf(D_LLVM_FILE, TAB "br label %%" PURPLE_LABEL_PREFIX "%llu" NEWLINE,
            label.value.label_index);
}

/**
 * @brief Generate a conditional jump statement
 * 
 * @param condition_register LLVMValue holding information about the register from the prior condition
 * @param true_label Label to jump to if condition is true
 * @param false_label Label to jump to if condition is false
 */
void llvm_conditional_jump(LLVMValue condition_register, LLVMValue true_label,
                           LLVMValue false_label)
{
    fprintf(D_LLVM_FILE,
            TAB "br %s %%%llu, label %%" PURPLE_LABEL_PREFIX "%llu, label %%" PURPLE_LABEL_PREFIX
                "%llu" NEWLINE,
            numberTypeLLVMReprs[condition_register.number_type],
            condition_register.value.virtual_register_index, true_label.value.label_index,
            false_label.value.label_index);
}

void llvm_function_preamble(char* symbol_name)
{
    SymbolTableEntry* entry = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, symbol_name);
    if (!entry) {
        fatal(RC_COMPILER_ERROR,
              "llvm_call_function received symbol name \"%s\", which is not an identifier",
              symbol_name);
    } else if (!entry->type.is_function) {
        fatal(RC_COMPILER_ERROR,
              "llvm_call_function received an identifier name that is not a function: \"%s\"",
              symbol_name);
    }

    D_CURRENT_FUNCTION_PREAMBLE_PRINTED = true;
    fprintf(D_LLVM_FILE, "define dso_local %s @%s() #0 {" NEWLINE,
            type_to_llvm_type(entry->type.value.function.return_type), symbol_name);

    // Print our buffered stack entries
    if (buffered_stack_entries_head != NULL) {
        llvm_stack_allocation(buffered_stack_entries_head);
        free_llvm_stack_entry_node_list(buffered_stack_entries_head);
        buffered_stack_entries_head = NULL;
    }
}

void llvm_function_postamble(void)
{
    fprintf(D_LLVM_FILE, "}" NEWLINE NEWLINE);
}

const char* type_to_llvm_type(TokenType type)
{
    if (type == T_VOID) {
        return "void";
    } else {
        return numberTypeLLVMReprs[token_type_to_number_type(type)];
    }
}

LLVMValue llvm_call_function(LLVMValue virtual_register, char* symbol_name)
{
    LLVMValue out = LLVMVALUE_NULL;

    SymbolTableEntry* entry = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, symbol_name);
    if (!entry) {
        fatal(RC_COMPILER_ERROR,
              "llvm_call_function received symbol name \"%s\", which is not an identifier",
              symbol_name);
    } else if (!entry->type.is_function) {
        fatal(RC_COMPILER_ERROR,
              "llvm_call_function received an identifier name that is not a function: \"%s\"",
              symbol_name);
    }

    fprintf(D_LLVM_FILE, TAB);

    if (entry->type.value.function.return_type != T_VOID) {
        out = LLVMVALUE_VIRTUAL_REGISTER(
            get_next_local_virtual_register(),
            token_type_to_number_type(entry->type.value.function.return_type));
        fprintf(D_LLVM_FILE, "%%%llu = ", out.value.virtual_register_index);
    }

    fprintf(D_LLVM_FILE, "call %s () @%s()" NEWLINE,
            type_to_llvm_type(entry->type.value.function.return_type), symbol_name);

    prepend_loaded(out.value.virtual_register_index);

    return out;
}

void llvm_return(LLVMValue value, char* symbol_name)
{
    SymbolTableEntry* entry = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, symbol_name);
    if (!entry) {
        fatal(RC_COMPILER_ERROR,
              "llvm_call_function received symbol name \"%s\", which is not an identifier",
              symbol_name);
    } else if (!entry->type.is_function) {
        fatal(RC_COMPILER_ERROR,
              "llvm_call_function received an identifier name that is not a function: \"%s\"",
              symbol_name);
    }

    if (value.value_type != LLVMVALUETYPE_CONSTANT &&
        entry->type.value.function.return_type != T_VOID) {

        type_register* loaded_registers = llvm_ensure_registers_loaded(
            1, (type_register[]){value.value.virtual_register_index}, value.number_type);
        if (loaded_registers != NULL) {
            value.value.virtual_register_index = loaded_registers[0];
            purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers", "llvm_return");
            free(loaded_registers);
        }
    }

    fprintf(D_LLVM_FILE, TAB "ret %s", type_to_llvm_type(entry->type.value.function.return_type));

    if (entry->type.value.function.return_type != T_VOID) {
        switch (value.value_type) {
        case LLVMVALUETYPE_CONSTANT:
            fprintf(D_LLVM_FILE, " %llu", value.value.constant);
            break;
        case LLVMVALUETYPE_VIRTUAL_REGISTER:
            fprintf(D_LLVM_FILE, " %%%llu", value.value.virtual_register_index);
            break;
        default:
            fatal(RC_COMPILER_ERROR, "llvm_return got LLVMValueType %d", value.value_type);
        }
    } else if (strcmp("main", symbol_name) == 0) {
        purple_log(LOG_WARNING, "Change \"main\" function return type to int");
    }

    fprintf(D_LLVM_FILE, NEWLINE);

    D_CURRENT_FUNCTION_HAS_RETURNED = true;
}
