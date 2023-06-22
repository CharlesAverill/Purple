/**
 * @file llvm.c
 * @author Charles Averill
 * @brief LLVM-IR emission
 * @date 12-Sep-2022
 */

#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "translate/llvm.h"
#include "translate/llvm_stack_entry.h"
#include "translate/translate.h"
#include "types/type.h"
#include "utils/clang.h"
#include "utils/formatting.h"
#include "utils/logging.h"

static void print_function_annotation(const char* function_name)
{
    if (D_ARGS->print_func_annotations) {
        fprintf(D_LLVM_FILE, TAB "; %s" NEWLINE, function_name);
    }
}

/**
 * @brief Gets the LLVM string representation of a Number struct, max length = 300
 * 
 * @param number    Number to represent
 * @return char*    calloc'd string
 */
static char* number_string(Number number)
{
    char* out = (char*)calloc(1, sizeof(char) * 300);
    sprintf(out, "%s%s", numberTypeLLVMReprs[number.number_type], REFSTRING(number.pointer_depth));
    return out;
}

/**
 * @brief Gets the LLVM string representation of an LLVMValue struct, max length = 600
 * 
 * @param val       LLVMValue to represent
 * @return char*    calloc'd string
 */
static char* llvmvalue_repr(LLVMValue val)
{
    char* out = (char*)calloc(1, sizeof(char) * 600);
    char* numstring = number_string(val.num_info);
    sprintf(out, "%s %s", numstring, LLVMVALUE_REGMARKER(val));
    free(numstring);
    if (val.has_name) {
        sprintf(out + strlen(out), "%s", val.value.name);
    } else {
        sprintf(out + strlen(out), "%llu", val.value.virtual_register_index);
    }
    return out;
}

/**
 * @brief Ensure that the values of a set of registers are loaded
 * 
 * @param n_registers Number of registers to ensure
 * @param registers Array of register indices to ensure
 * @param number_type NumberType of registers to ensure
 * @param load_depth Pointer depth to be considered "loaded"
 * @return LLVMValue* If the registers were not loaded, this array contains the loaded registers
 */
LLVMValue* llvm_ensure_registers_loaded(int n_registers, LLVMValue registers[], int load_depth)
{
    bool found_registers[n_registers];
    int n_found = 0;
    for (int i = 0; i < n_registers; i++) {
        found_registers[i] = false;
    }

    for (int i = 0; i < n_registers; i++) {
        if (registers[i].num_info.pointer_depth <= load_depth ||
            registers[i].value_type != LLVMVALUETYPE_VIRTUAL_REGISTER) {
            found_registers[i] = true;
            n_found++;
        }

        if (n_found >= n_registers) {
            return NULL;
        }
    }

    // Haven't loaded all of our registers yet
    print_function_annotation("llvm_ensure_registers_loaded");
    LLVMValue* loaded_registers = (LLVMValue*)malloc(sizeof(LLVMValue) * (n_registers - n_found));
    for (int i = 0; i < n_registers; i++) {
        if (found_registers[i]) {
            loaded_registers[i] = registers[i];
        } else {
            type_register last_loaded_reg;
            bool has_loaded_once = false;
            for (int j = registers[i].num_info.pointer_depth; j > load_depth; j--) {
                int new_reg = get_next_local_virtual_register();
                loaded_registers[i] = LLVMVALUE_VIRTUAL_REGISTER_POINTER(
                    new_reg, registers[i].num_info.number_type, j - 1);
                fprintf(D_LLVM_FILE, TAB "%%%llu = load %s%s, ",
                        loaded_registers[i].value.virtual_register_index,
                        numberTypeLLVMReprs[registers[i].num_info.number_type],
                        REFSTRING(loaded_registers[i].num_info.pointer_depth));
                if (has_loaded_once) {
                    fprintf(D_LLVM_FILE, "%s%s %%%llu, align %d" NEWLINE,
                            numberTypeLLVMReprs[registers[i].num_info.number_type], REFSTRING(j),
                            last_loaded_reg, numberTypeByteSizes[i]);
                } else {
                    fprintf(D_LLVM_FILE, "%s%s %s, align %d" NEWLINE,
                            numberTypeLLVMReprs[registers[i].num_info.number_type], REFSTRING(j),
                            LLVM_REPR_NOTYPE(registers[i]), numberTypeByteSizes[i]);
                }

                has_loaded_once = true;
                last_loaded_reg = new_reg;
            }
        }
    }

    return loaded_registers;
}

/**
 * @brief Overloaded version of llvm_ensure_registers_loaded where load_depth=0
 */
LLVMValue* llvm_ensure_registers_fully_loaded(int n_registers, LLVMValue registers[])
{
    return llvm_ensure_registers_loaded(n_registers, registers, 0);
}

/**
 * @brief Generated program's preamble
 */
void llvm_preamble(void)
{
    print_function_annotation("llvm_preamble");
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
    print_function_annotation("llvm_postamble");
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

    if (current) {
        print_function_annotation("llvm_stack_allocation");
    }
    while (current) {
        fprintf(D_LLVM_FILE, TAB "%%%llu = alloca %s%s, align %d" NEWLINE,
                current->reg.value.virtual_register_index,
                numberTypeLLVMReprs[current->reg.num_info.number_type],
                REFSTRING(current->reg.num_info.pointer_depth), current->align_bytes);
        current = current->next;
    }

    return true;
}

/**
 * @brief Generate code for binary addition
 * 
 * @param left_virtual_register Lvalue to be added
 * @param right_virtual_register Rvalue to be added
 * @return LLVMValue Virtual register holding result
 */
static LLVMValue llvm_add(LLVMValue left_virtual_register, LLVMValue right_virtual_register)
{
    print_function_annotation("llvm_add");
    fprintf(D_LLVM_FILE, TAB "%%%llu = add nsw %s %s, ", get_next_local_virtual_register(),
            numberTypeLLVMReprs[left_virtual_register.num_info.number_type],
            LLVM_REPR_NOTYPE(left_virtual_register));
    fprintf(D_LLVM_FILE, "%s" NEWLINE, LLVM_REPR_NOTYPE(right_virtual_register));
    return LLVMVALUE_VIRTUAL_REGISTER(D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER - 1,
                                      left_virtual_register.num_info.number_type);
}

/**
 * @brief Generate code for binary subtraction
 * 
 * @param left_virtual_register Lvalue to be subtracted
 * @param right_virtual_register Rvalue to be subtracted
 * @return LLVMValue Virtual register holding result
 */
static LLVMValue llvm_subtract(LLVMValue left_virtual_register, LLVMValue right_virtual_register)
{
    print_function_annotation("llvm_subtract");
    fprintf(D_LLVM_FILE, TAB "%%%llu = sub nsw %s %s, ", get_next_local_virtual_register(),
            numberTypeLLVMReprs[left_virtual_register.num_info.number_type],
            LLVM_REPR_NOTYPE(left_virtual_register));
    fprintf(D_LLVM_FILE, "%s" NEWLINE, LLVM_REPR_NOTYPE(right_virtual_register));
    return LLVMVALUE_VIRTUAL_REGISTER(D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER - 1,
                                      left_virtual_register.num_info.number_type);
}

/**
 * @brief Generate code for binary multiplication
 * 
 * @param left_virtual_register Lvalue to be multiplied
 * @param right_virtual_register Rvalue to be multiplied
 * @return LLVMValue Virtual register holding result
 */
static LLVMValue llvm_multiply(LLVMValue left_virtual_register, LLVMValue right_virtual_register)
{
    print_function_annotation("llvm_multiply");
    fprintf(D_LLVM_FILE, TAB "%%%llu = mul nsw %s %s, ", get_next_local_virtual_register(),
            numberTypeLLVMReprs[left_virtual_register.num_info.number_type],
            LLVM_REPR_NOTYPE(left_virtual_register));
    fprintf(D_LLVM_FILE, "%s" NEWLINE, LLVM_REPR_NOTYPE(right_virtual_register));
    return LLVMVALUE_VIRTUAL_REGISTER(D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER - 1,
                                      left_virtual_register.num_info.number_type);
}

/**
 * @brief Generate code for unsigned binary division
 * 
 * @param left_virtual_register Lvalue to be divided
 * @param right_virtual_register Rvalue to be divided
 * @return LLVMValue Virtual register holding result
 */
static LLVMValue llvm_divide(LLVMValue left_virtual_register, LLVMValue right_virtual_register)
{
    fprintf(D_LLVM_FILE, TAB "%%%llu = udiv %s %s, ", get_next_local_virtual_register(),
            numberTypeLLVMReprs[left_virtual_register.num_info.number_type],
            LLVM_REPR_NOTYPE(left_virtual_register));
    fprintf(D_LLVM_FILE, "%s" NEWLINE, LLVM_REPR_NOTYPE(right_virtual_register));
    return LLVMVALUE_VIRTUAL_REGISTER(D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER - 1,
                                      left_virtual_register.num_info.number_type);
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
    LLVMValue out_register;

    if (left_virtual_register.value_type == LLVMVALUETYPE_CONSTANT &&
        right_virtual_register.value_type == LLVMVALUETYPE_CONSTANT) {
        out_register = LLVMVALUE_CONSTANT(0);
        switch (operation) {
        case T_PLUS:
            out_register.value.constant =
                left_virtual_register.value.constant + right_virtual_register.value.constant;
            break;
        case T_MINUS:
            out_register.value.constant =
                left_virtual_register.value.constant - right_virtual_register.value.constant;
            break;
        case T_STAR:
            out_register.value.constant =
                left_virtual_register.value.constant * right_virtual_register.value.constant;
            break;
        case T_SLASH:
            out_register.value.constant =
                left_virtual_register.value.constant / right_virtual_register.value.constant;
            break;
        case T_EXPONENT:
            out_register.value.constant = (int)pow(left_virtual_register.value.constant,
                                                   right_virtual_register.value.constant);
            break;
        default:
            fatal(RC_COMPILER_ERROR,
                  "Can't perform compile-time reduction of constant integer values on operation "
                  "\'%s\'",
                  tokenStrings[operation]);
        }

        out_register.num_info.number_type = MIN(left_virtual_register.num_info.number_type,
                                                right_virtual_register.num_info.number_type);
        while (out_register.value.constant >
               numberTypeMaxValues[out_register.num_info.number_type]) {
            out_register.num_info.number_type++;
        }

        return out_register;
    }

    LLVMValue* loaded_registers =
        llvm_ensure_registers_fully_loaded(1, (LLVMValue[]){left_virtual_register});
    if (loaded_registers != NULL) {
        left_virtual_register = loaded_registers[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers (1)", "llvm_binary_arithmetic");
        free(loaded_registers);
        loaded_registers = NULL;
    }

    loaded_registers = llvm_ensure_registers_fully_loaded(1, (LLVMValue[]){right_virtual_register});
    if (loaded_registers != NULL) {
        right_virtual_register = loaded_registers[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers (2)", "llvm_binary_arithmetic");
        free(loaded_registers);
    }

    if (left_virtual_register.num_info.number_type != right_virtual_register.num_info.number_type) {
        if (left_virtual_register.num_info.number_type <
            right_virtual_register.num_info.number_type) {
            left_virtual_register =
                llvm_int_resize(left_virtual_register, right_virtual_register.num_info.number_type);
        } else {
            right_virtual_register =
                llvm_int_resize(right_virtual_register, left_virtual_register.num_info.number_type);
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

    return out_register;
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
    print_function_annotation("llvm_store_constant");
    type_register out_register_number = pop_stack_entry_linked_list(&freeVirtualRegistersHead);
    fprintf(D_LLVM_FILE, TAB "store %s ", numberTypeLLVMReprs[value.number_type]);
    fprintf(D_LLVM_FILE, numberTypeFormatStrings[value.number_type], value.value);
    fprintf(D_LLVM_FILE, ", %s* %%%llu, align %d" NEWLINE, numberTypeLLVMReprs[value.number_type],
            out_register_number, numberTypeByteSizes[value.number_type]);
    return LLVMVALUE_VIRTUAL_REGISTER_POINTER(out_register_number, value.number_type, 1);
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

    SymbolTableEntry* symbol = STS_FIND(symbol_name);
    if (symbol == NULL) {
        fatal(RC_COMPILER_ERROR, "Failed to find symbol \"%s\" in Global Symbol Table",
              symbol_name);
    }

    print_function_annotation("llvm_load_global_variable");

    fprintf(D_LLVM_FILE, TAB "%%%llu = load %s%s, ", out_register_number,
            numberTypeLLVMReprs[symbol->type.value.number.number_type],
            REFSTRING(symbol->type.value.number.pointer_depth - 1));
    fprintf(D_LLVM_FILE, "%s%s @%s" NEWLINE,
            numberTypeLLVMReprs[symbol->type.value.number.number_type],
            REFSTRING(symbol->type.value.number.pointer_depth), symbol_name);

    LLVMValue out = LLVMVALUE_VIRTUAL_REGISTER_POINTER(out_register_number,
                                                       symbol->type.value.number.number_type,
                                                       symbol->type.value.number.pointer_depth - 1);

    LLVMVALUE_SET_JUSTLOADED(out, symbol_name);
    return out;
}

/**
 * @brief Store a value into a global variable
 * 
 * @param symbol_name Identifier name of variable to store new value to
 * @param rvalue_register Register number of statement's RValue to store
 */
void llvm_store_global_variable(char* symbol_name, LLVMValue rvalue_register)
{
    printf("%s %d\n", rvalue_register.just_loaded, rvalue_register.just_loaded);
    if (rvalue_register.value_type != LLVMVALUETYPE_CONSTANT &&
        rvalue_register.value_type != LLVMVALUETYPE_VIRTUAL_REGISTER) {
        fatal(RC_COMPILER_ERROR, "Non-value passed to llvm_store_global_variable");
    }

    SymbolTableEntry* symbol = STS_FIND(symbol_name);
    if (symbol == NULL) {
        fatal(RC_COMPILER_ERROR, "Failed to find symbol \"%s\" in Global Symbol Table",
              symbol_name);
    }

    if (rvalue_register.value_type == LLVMVALUETYPE_VIRTUAL_REGISTER) {
        LLVMValue* loaded_registers = llvm_ensure_registers_loaded(
            1, (LLVMValue[]){rvalue_register}, symbol->type.value.number.pointer_depth - 1);
        if (loaded_registers) {
            rvalue_register = loaded_registers[0];
            purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers",
                       "llvm_store_global_variable");
            free(loaded_registers);
        }
        if (rvalue_register.num_info.pointer_depth != symbol->type.value.number.pointer_depth - 1) {
            fatal(RC_COMPILER_ERROR, "Pointer mismatch when trying to save global variable");
        }
    }

    if (TOKENTYPE_IS_NUMBER_TYPE(symbol->type.token_type) &&
        rvalue_register.num_info.number_type != symbol->type.value.number.number_type) {
        if (numberTypeBitSizes[rvalue_register.num_info.number_type] !=
            numberTypeBitSizes[symbol->type.value.number.number_type]) {
            rvalue_register =
                llvm_int_resize(rvalue_register, symbol->type.value.number.number_type);
        }
    }

    print_function_annotation("llvm_store_global_variable");
    if (rvalue_register.just_loaded[0] == 0) {
        fprintf(D_LLVM_FILE, TAB "store %s%s %s, ",
                numberTypeLLVMReprs[symbol->type.value.number.number_type],
                REFSTRING(rvalue_register.num_info.pointer_depth),
                LLVM_REPR_NOTYPE(rvalue_register));
    } else {
        printf("JUST LOADED: %s %d\n", rvalue_register.just_loaded, rvalue_register.just_loaded);
        fprintf(D_LLVM_FILE, TAB "store %s%s @%s, ",
                numberTypeLLVMReprs[symbol->type.value.number.number_type],
                REFSTRING(rvalue_register.num_info.pointer_depth), rvalue_register.just_loaded);
    }

    fprintf(D_LLVM_FILE, "%s%s @%s" NEWLINE,
            numberTypeLLVMReprs[symbol->type.value.number.number_type],
            REFSTRING(symbol->type.value.number.pointer_depth), symbol_name);
}

/**
 * @brief Generates an extend or truncate statement to change the bit-width of an integer
 * 
 * @param reg           Register whose contents are to be resized
 * @param new_type      New NumberType to resize to
 * @return LLVMValue    Resized LLVMValue
 */
LLVMValue llvm_int_resize(LLVMValue reg, NumberType new_type)
{
    if (reg.value_type == LLVMVALUETYPE_CONSTANT) {
        reg.value.constant = MIN(reg.value.constant, numberTypeMaxValues[new_type]);
        reg.num_info.number_type = new_type;
        return reg;
    } else if (reg.value_type == LLVMVALUETYPE_NONE) {
        fatal(RC_COMPILER_ERROR, "llvm_int_resize tried to resize a null LLVMValue");
    }

    print_function_annotation("llvm_int_resize");

    char* method;

    if (reg.num_info.number_type < new_type) {
        method = "zext";
    } else if (reg.num_info.number_type > new_type) {
        method = "trunc";
    } else {
        return reg;
    }

    LLVMValue out = LLVMVALUE_VIRTUAL_REGISTER(get_next_local_virtual_register(), new_type);

    fprintf(D_LLVM_FILE, TAB "%%%llu = %s %s %%%llu to %s" NEWLINE,
            out.value.virtual_register_index, method, numberTypeLLVMReprs[reg.num_info.number_type],
            reg.value.virtual_register_index, numberTypeLLVMReprs[new_type]);

    return out;
}

/**
 * @brief Declare a global variable
 * 
 * @param symbol_name Name of global variable
 * @param n Number information of global variable
 */
void llvm_declare_global_number_variable(char* symbol_name, Number n)
{
    fprintf(D_LLVM_GLOBALS_FILE, "@%s = global %s%s ", symbol_name,
            numberTypeLLVMReprs[n.number_type], REFSTRING(n.pointer_depth - 1));
    if (n.pointer_depth - 1 <= 0) {
        fprintf(D_LLVM_GLOBALS_FILE, "%lld" NEWLINE, n.value);
    } else {
        fprintf(D_LLVM_GLOBALS_FILE, "null" NEWLINE);
    }
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
            numberTypeLLVMReprs[number.number_type], number.value);
}

LLVMValue llvm_declare_local(char* symbol_name, Number num)
{
    LLVMValue out = (LLVMValue){.has_name = true, .num_info = num};
    strcpy(out.value.name, symbol_name);

    llvm_stack_allocation((LLVMStackEntryNode[]){(LLVMStackEntryNode){.reg = out, .next = NULL}});

    out.num_info.pointer_depth++;

    if (num.value != 0) {
        llvm_store_local_llvmvalue(out, LLVMVALUE_CONSTANT(num.value));
    }

    return out;
}

/**
 * @brief Generate code to print an integer
 * 
 * @param print_vr Register holding value to print
 */
void llvm_print_int(LLVMValue print_vr)
{
    LLVMValue* loaded_register = llvm_ensure_registers_fully_loaded(1, (LLVMValue[]){print_vr});
    if (loaded_register) {
        print_vr = loaded_register[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_register", "llvm_print_int");
        free(loaded_register);
    }

    print_function_annotation("llvm_print_int");

    get_next_local_virtual_register();
    switch (print_vr.num_info.number_type) {
    case NT_INT8:
    case NT_INT16:
    case NT_INT32:
        fprintf(D_LLVM_FILE,
                TAB "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x "
                    "i8]* @print_int_fstring , i32 0, i32 0), %s %s)" NEWLINE,
                numberTypeLLVMReprs[print_vr.num_info.number_type], LLVM_REPR_NOTYPE(print_vr));
        break;
    case NT_INT64:
        fprintf(D_LLVM_FILE,
                TAB "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x "
                    "i8]* @print_long_fstring , i32 0, i32 0), %s %s)" NEWLINE,
                numberTypeLLVMReprs[NT_INT64], LLVM_REPR_NOTYPE(print_vr));
        break;
    default:
        fatal(RC_COMPILER_ERROR, "Unrecognized NumberType %s",
              numberTypeNames[print_vr.num_info.number_type]);
    }
}

/**
 * @brief Generate code to print a boolean value
 * 
 * @param print_vr Register holding value to print
 */
void llvm_print_bool(LLVMValue print_vr)
{
    LLVMValue* loaded_register = llvm_ensure_registers_fully_loaded(1, (LLVMValue[]){print_vr});
    if (loaded_register) {
        print_vr = loaded_register[0];
    }

    print_function_annotation("llvm_print_bool");

    LLVMValue compare_register = print_vr;
    LLVMValue true_label, false_label, end_label;
    true_label = get_next_label();
    false_label = get_next_label();
    end_label = get_next_label();

    llvm_conditional_jump(compare_register, true_label, false_label);
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

/**
 * @brief Generates a relational (inline) comparison statement
 * 
 * @param comparison_type           What kind of comparison to generate
 * @param out_register              Register to store output into
 * @param left_virtual_register     Left comparison operand
 * @param right_virtual_register    Right comparison operand
 */
static void llvm_relational_compare(TokenType comparison_type, LLVMValue out_register,
                                    LLVMValue left_virtual_register,
                                    LLVMValue right_virtual_register)
{
    print_function_annotation("llvm_relational_compare");
    fprintf(D_LLVM_FILE, TAB "%%%llu = icmp ", out_register.value.virtual_register_index);

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

    fprintf(D_LLVM_FILE, " %s %s, ",
            numberTypeLLVMReprs[left_virtual_register.num_info.number_type],
            LLVM_REPR_NOTYPE(left_virtual_register));
    fprintf(D_LLVM_FILE, "%s" NEWLINE, LLVM_REPR_NOTYPE(right_virtual_register));
}

/**
 * @brief Generates a logical (statement-level) comparison statement
 * 
 * @param comparison_type           What kind of comparison to generate
 * @param out_register              Register to store output into
 * @param left_virtual_register     Left comparison operand
 * @param right_virtual_register    Right comparison operand
 */
static void llvm_logical_compare(TokenType comparison_type, LLVMValue out_register,
                                 LLVMValue left_virtual_register, LLVMValue right_virtual_register)
{
    print_function_annotation("llvm_logical_compare");
    fprintf(D_LLVM_FILE, TAB "%%%llu = ", out_register.value.virtual_register_index);

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

    fprintf(D_LLVM_FILE, " %s %s, ",
            numberTypeLLVMReprs[left_virtual_register.num_info.number_type],
            LLVM_REPR_NOTYPE(left_virtual_register));
    fprintf(D_LLVM_FILE, "%s" NEWLINE, LLVM_REPR_NOTYPE(right_virtual_register));
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
    LLVMValue* loaded_registers =
        llvm_ensure_registers_fully_loaded(1, (LLVMValue[]){left_virtual_register});
    if (loaded_registers != NULL) {
        left_virtual_register = loaded_registers[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers", "llvm_compare");
        free(loaded_registers);
    }

    loaded_registers = llvm_ensure_registers_fully_loaded(1, (LLVMValue[]){right_virtual_register});
    if (loaded_registers != NULL) {
        right_virtual_register = loaded_registers[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers", "llvm_compare");
        free(loaded_registers);
        loaded_registers = NULL;
    }

    if (left_virtual_register.num_info.number_type != right_virtual_register.num_info.number_type) {
        if (numberTypeBitSizes[left_virtual_register.num_info.number_type] <
            numberTypeBitSizes[right_virtual_register.num_info.number_type]) {
            left_virtual_register =
                llvm_int_resize(left_virtual_register, right_virtual_register.num_info.number_type);
        } else {
            right_virtual_register =
                llvm_int_resize(right_virtual_register, left_virtual_register.num_info.number_type);
        }
    }

    if (left_virtual_register.value_type == LLVMVALUETYPE_CONSTANT &&
        right_virtual_register.value_type == LLVMVALUETYPE_CONSTANT) {
        LLVMValue out = LLVMVALUE_CONSTANT(0);
        out.num_info.number_type = NT_INT1;

        switch (comparison_type) {
        case T_EQ:
            out.value.constant =
                left_virtual_register.value.constant == right_virtual_register.value.constant;
            break;
        case T_NEQ:
            out.value.constant =
                left_virtual_register.value.constant != right_virtual_register.value.constant;
            break;
        case T_LT:
            out.value.constant =
                left_virtual_register.value.constant < right_virtual_register.value.constant;
            break;
        case T_LE:
            out.value.constant =
                left_virtual_register.value.constant <= right_virtual_register.value.constant;
            break;
        case T_GT:
            out.value.constant =
                left_virtual_register.value.constant > right_virtual_register.value.constant;
            break;
        case T_GE:
            out.value.constant =
                left_virtual_register.value.constant >= right_virtual_register.value.constant;
            break;
        case T_AND:
            out.value.constant =
                left_virtual_register.value.constant && right_virtual_register.value.constant;
            break;
        case T_OR:
            out.value.constant =
                left_virtual_register.value.constant || right_virtual_register.value.constant;
            break;
        case T_XOR:
            out.value.constant =
                left_virtual_register.value.constant ^ right_virtual_register.value.constant;
            break;
        case T_NAND:
            out.value.constant =
                !(left_virtual_register.value.constant && right_virtual_register.value.constant);
            break;
        case T_NOR:
            out.value.constant =
                !(left_virtual_register.value.constant || right_virtual_register.value.constant);
            break;
        case T_XNOR:
            out.value.constant =
                !(left_virtual_register.value.constant ^ right_virtual_register.value.constant);
            break;
        default:
            fatal(RC_COMPILER_ERROR,
                  "Can't perform compile-time reduction of constant integer values on operation "
                  "\'%s\'",
                  tokenStrings[comparison_type]);
        }

        return out;
    }

    LLVMValue out_register = LLVMVALUE_VIRTUAL_REGISTER(get_next_local_virtual_register(), NT_INT1);

    if (TOKENTYPE_IS_COMPARATOR(comparison_type)) {
        llvm_relational_compare(comparison_type, out_register, left_virtual_register,
                                right_virtual_register);
    } else {
        llvm_logical_compare(comparison_type, out_register, left_virtual_register,
                             right_virtual_register);
    }

    return out_register;
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

    // if (comparison_result.value_type != LLVMVALUETYPE_VIRTUAL_REGISTER) {
    //     fatal(RC_COMPILER_ERROR, "Tried to generate jump after comparison, but comparison did not "
    //                              "return a virtual register number");
    // }

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

    print_function_annotation("llvm_label");

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

    print_function_annotation("llvm_jump");

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
    print_function_annotation("llvm_conditional_jump");
    fprintf(D_LLVM_FILE,
            TAB "br %s %s, label %%" PURPLE_LABEL_PREFIX "%llu, label %%" PURPLE_LABEL_PREFIX
                "%llu" NEWLINE,
            numberTypeLLVMReprs[condition_register.num_info.number_type],
            LLVM_REPR_NOTYPE(condition_register), true_label.value.label_index,
            false_label.value.label_index);
}

/**
 * @brief Generates the preamble for a function
 * 
 * @param symbol_name   Name of function to generate for
 * @return LLVMValue*   List of LLVMValues corresponding to the latest_llvmvalues for each function input
 */
LLVMValue* llvm_function_preamble(char* symbol_name)
{
    SymbolTableEntry* entry = STS_FIND(symbol_name);
    if (!entry) {
        fatal(RC_COMPILER_ERROR,
              "llvm_function_preamble received symbol name \"%s\", which is not an identifier",
              symbol_name);
    } else if (!entry->type.is_function) {
        fatal(RC_COMPILER_ERROR,
              "llvm_function_preamble received an identifier name that is not a function: \"%s\"",
              symbol_name);
    }

    // Generate a string containing all arguments, comma-separated by looping
    // through the expected number of arguments and pulling their types one by
    // one
    int args_size = 256;
    char* args_str = (char*)calloc(1, sizeof(char) * args_size);
    for (int i = 0; i < entry->type.value.function.num_parameters; i++) {
        // Sprintf into a temporary buffer first, so that we can make sure it won't
        // overflow the main buffer
        char* curr_arg_str = (char*)calloc(1, 300);
        sprintf(
            curr_arg_str, "%s%s %%%llu%s",
            numberTypeLLVMReprs[entry->type.value.function.parameters[i]
                                    .parameter_type.number_type],
            REFSTRING(entry->type.value.function.parameters[i].parameter_type.pointer_depth - 1),
            get_next_local_virtual_register() - 1,
            i != entry->type.value.function.num_parameters - 1 ? ", " : "");
        // If it does want to overflow, resize the main buffer
        while (strlen(args_str) + strlen(curr_arg_str) > args_size) {
            args_size += 256;
            args_str = realloc(args_str, args_size);
        }
        strcat(args_str, curr_arg_str);
        free(curr_arg_str);
    }

    print_function_annotation("llvm_function_preamble");

    fprintf(D_LLVM_FILE, "define dso_local %s @%s(%s) #0 {" NEWLINE,
            type_to_llvm_type(entry->type.value.function.return_type), symbol_name, args_str);

    free(args_str);

    // Print our buffered stack entries
    if (buffered_stack_entries_head != NULL) {
        llvm_stack_allocation(buffered_stack_entries_head);
        free_llvm_stack_entry_node_list(&buffered_stack_entries_head);
        buffered_stack_entries_head = NULL;
    }

    D_CURRENT_FUNCTION_PREAMBLE_PRINTED = true;

    // Build a list of LLVMValues as they're generated
    LLVMValue* arguments_llvmvalues =
        (LLVMValue*)malloc(sizeof(LLVMValue) * entry->type.value.function.num_parameters);
    for (unsigned long long int i = 0; i < entry->type.value.function.num_parameters; i++) {
        Number param_num = entry->type.value.function.parameters[i].parameter_type;
        // Not needed yet? Not sure why
        char* numstring = number_string(param_num);
        fprintf(D_LLVM_FILE, TAB "%%%s = alloca %s%s, align %d" NEWLINE,
                entry->type.value.function.parameters[i].parameter_name,
                numberTypeLLVMReprs[param_num.number_type], REFSTRING(param_num.pointer_depth - 1),
                numberTypeByteSizes[param_num.number_type]);
        fprintf(D_LLVM_FILE, TAB "store %s%s %%%llu, %s%s* %%%s" NEWLINE,
                numberTypeLLVMReprs[param_num.number_type], REFSTRING(param_num.pointer_depth - 1),
                i, numberTypeLLVMReprs[param_num.number_type], _refstring_buf,
                entry->type.value.function.parameters[i].parameter_name);
        arguments_llvmvalues[i] = (LLVMValue){
            .value_type = LLVMVALUETYPE_VIRTUAL_REGISTER, .num_info = param_num, .has_name = true};
        arguments_llvmvalues[i].num_info.pointer_depth += 1;
        strcpy(arguments_llvmvalues[i].value.name,
               entry->type.value.function.parameters[i].parameter_name);

        SymbolTableEntry* ste = STS_FIND(arguments_llvmvalues[i].value.name);
        if (ste) {
            ste->latest_llvmvalue = arguments_llvmvalues[i];
        }
    }

    return arguments_llvmvalues;
}

/**
 * @brief Generates the postamble for a function
 */
void llvm_function_postamble(void)
{
    print_function_annotation("llvm_function_postamble");
    fprintf(D_LLVM_FILE, "}" NEWLINE NEWLINE);
}

/**
 * @brief Convert a TokenType to the string representation of that type in LLVM
 * 
 * @param type          TokenType to convert
 * @return const char*  Matching LLVM type string
 */
const char* type_to_llvm_type(TokenType type)
{
    if (type == T_VOID) {
        return "void";
    } else {
        return numberTypeLLVMReprs[token_type_to_number_type(type)];
    }
}

/**
 * @brief Generate a function call statement
 * 
 * @param args              Currently unused function parameter
 * @param num_args          Number of args passed
 * @param symbol_name       Name of function to call
 * @return LLVMValue        Output of function, or LLVMVALUE_NULL if it is a void function
 */
LLVMValue llvm_call_function(LLVMValue* args, unsigned long long int num_args, char* symbol_name)
{
    LLVMValue out = LLVMVALUE_NULL;

    SymbolTableEntry* entry = STS_FIND(symbol_name);
    if (!entry) {
        fatal(RC_COMPILER_ERROR,
              "llvm_call_function received symbol name \"%s\", which is not an identifier",
              symbol_name);
    } else if (!entry->type.is_function) {
        fatal(RC_COMPILER_ERROR,
              "llvm_call_function received an identifier name that is not a function: \"%s\"",
              symbol_name);
    }

    // Build the strings for passing in types and values, check passed args against
    // expected args at the same time
    if (num_args != entry->type.value.function.num_parameters) {
        fatal(RC_COMPILER_ERROR,
              "Incorrect number of arguments to function call allowed to propagate to compilation "
              "phase, got %llu but expected %llu",
              num_args, entry->type.value.function.num_parameters);
    }
    int passed_types_size, passed_values_size = 256;
    char* passed_types = (char*)calloc(1, sizeof(char) * passed_types_size);
    char* passed_values = (char*)calloc(1, sizeof(char) * passed_values_size);
    for (unsigned long long int i = 0; i < num_args; i++) {
        FunctionParameter param = entry->type.value.function.parameters[i];

        LLVMValue* loaded_param = llvm_ensure_registers_loaded(1, (LLVMValue[]){args[i]},
                                                               param.parameter_type.pointer_depth);
        if (loaded_param) {
            args[i] = loaded_param[0];
            purple_log(LOG_DEBUG, "Freeing loaded_param in llvm_call_function");
            free(loaded_param);
            loaded_param = NULL;
        }

        if (!NUMBERS_TYPEQUIV(param.parameter_type, args[i].num_info)) {
            char expectedstrarr[300];
            char* expectedstr = number_string(param.parameter_type);
            strcpy(expectedstrarr, expectedstr);
            free(expectedstr);
            char gotstrarr[300];
            char* gotstr = number_string(args[i].num_info);
            strcpy(gotstrarr, gotstr);
            free(gotstr);

            fatal(RC_COMPILER_ERROR,
                  "Function '%s' expected parameter '%s' to have type '%s' but got '%s'",
                  symbol_name, param.parameter_name, expectedstrarr, gotstrarr);
        }

        char curr_typ_str[300];
        char* curr_arg_str = (char*)calloc(1, 300);
        char* numstring = number_string(param.parameter_type);
        char* argstring = llvmvalue_repr(args[i]);
        sprintf(curr_typ_str, "%s%s", numstring, i != num_args - 1 ? ", " : "");
        sprintf(curr_arg_str, "%s%s", argstring, i != num_args - 1 ? ", " : "");
        // If it does want to overflow, resize the main buffers
        while (strlen(passed_types) + strlen(curr_typ_str) > passed_types_size) {
            passed_types_size += 256;
            passed_types = realloc(passed_types, passed_types_size);
        }
        while (strlen(passed_values) + strlen(curr_arg_str) > passed_values_size) {
            passed_values_size += 256;
            passed_values = realloc(passed_values, passed_values_size);
        }
        strcat(passed_types, curr_typ_str);
        strcat(passed_values, curr_arg_str);
        free(numstring);
        free(argstring);
    }

    print_function_annotation("llvm_call_function");

    fprintf(D_LLVM_FILE, TAB);

    if (entry->type.value.function.return_type != T_VOID) {
        out = LLVMVALUE_VIRTUAL_REGISTER(
            get_next_local_virtual_register(),
            token_type_to_number_type(entry->type.value.function.return_type));
        fprintf(D_LLVM_FILE, "%%%llu = ", out.value.virtual_register_index);
    }

    fprintf(D_LLVM_FILE, "call %s (%s) @%s(%s)" NEWLINE,
            type_to_llvm_type(entry->type.value.function.return_type), passed_types, symbol_name,
            passed_values);

    return out;
}

/**
 * @brief Generate a return statement
 * 
 * @param value         Value to return
 * @param symbol_name   Name of function to return from
 */
void llvm_return(LLVMValue value, char* symbol_name)
{
    SymbolTableEntry* entry = STS_FIND(symbol_name);
    if (!entry) {
        fatal(RC_COMPILER_ERROR,
              "llvm_return received symbol name \"%s\", which is not an identifier", symbol_name);
    } else if (!entry->type.is_function) {
        fatal(RC_COMPILER_ERROR,
              "llvm_return received an identifier name that is not a function: \"%s\"",
              symbol_name);
    }

    if (value.value_type != LLVMVALUETYPE_CONSTANT &&
        entry->type.value.function.return_type != T_VOID) {

        LLVMValue* loaded_registers = llvm_ensure_registers_fully_loaded(1, (LLVMValue[]){value});
        if (loaded_registers != NULL) {
            value = loaded_registers[0];
            purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers", "llvm_return");
            free(loaded_registers);
        }
    }

    print_function_annotation("llvm_return");

    fprintf(D_LLVM_FILE, TAB "ret %s", type_to_llvm_type(entry->type.value.function.return_type));
    if (entry->type.value.function.return_type != T_VOID) {
        fprintf(D_LLVM_FILE, " %s", LLVM_REPR_NOTYPE(value));
    }

    fprintf(D_LLVM_FILE, NEWLINE);

    if (strcmp("main", symbol_name) == 0 && entry->type.value.function.return_type != T_INT) {
        purple_log(LOG_WARNING, "Change \"main\" function return type to int");
    }

    D_CURRENT_FUNCTION_HAS_RETURNED = true;
    get_next_local_virtual_register();
}

/**
 * @brief Generate a string containing pointer stars
 * 
 * @param buf               Buffer to fill with pointer stars
 * @param pointer_depth     Number of pointer stars to fill
 * @return char*            Resulting string (for inline use)
 */
char* refstring(char* buf, int pointer_depth)
{
    if (pointer_depth >= REFSTRING_BUF_MAXLEN) {
        fatal(RC_MEMORY_ERROR, "Tried to request a pointer star string too large");
    }

    int i;
    buf[0] = '\0';
    for (i = 0; i < pointer_depth; i++) {
        buf[i] = '*';
    }
    buf[i] = '\0';
    return buf;
}

char* llvmvalue_repr_notype(char* buf, LLVMValue reg)
{
    if (reg.value_type == LLVMVALUETYPE_NONE) {
        fatal(RC_COMPILER_ERROR, "Tried to generate llvm name for null LLVMValue");
    }

    buf[0] = '\0';
    strcat(buf, LLVMVALUE_REGMARKER(reg));

    if (reg.has_name) {
        strcat(buf, reg.value.name);
    } else if (reg.value_type == LLVMVALUETYPE_CONSTANT) {
        sprintf(buf + strlen(buf), "%lld", reg.value.constant);
    } else if (reg.value_type == LLVMVALUETYPE_VIRTUAL_REGISTER ||
               reg.value_type == LLVMVALUETYPE_LABEL) {
        sprintf(buf + strlen(buf), "%llu", reg.value.virtual_register_index);
    }

    return buf;
}

/**
 * @brief Generate an addressing statement
 * 
 * @param symbol_name   Symbol to take the address of
 * @return LLVMValue    LLVMValue containing address of symbol
 */
LLVMValue llvm_get_address(char* symbol_name)
{
    SymbolTableEntry* entry = STS_FIND(symbol_name);
    if (!entry) {
        fatal(RC_COMPILER_ERROR,
              "llvm_get_address received symbol name \"%s\", which is not an identifier",
              symbol_name);
    } else if (entry->type.is_function) {
        fatal(RC_COMPILER_ERROR,
              "llvm_get_address received an identifier name that is not a number: \"%s\"",
              symbol_name);
    }

    type_register free_reg = get_next_local_virtual_register();
    LLVMValue lv = LLVMVALUE_VIRTUAL_REGISTER_POINTER(
        free_reg, entry->type.value.number.number_type, entry->type.value.number.pointer_depth);

    llvm_stack_allocation(
        (LLVMStackEntryNode[]){(LLVMStackEntryNode){.reg = lv, .align_bytes = 4}});

    lv.num_info.pointer_depth++;

    print_function_annotation("llvm_get_address");

    fprintf(D_LLVM_FILE, TAB "store %s%s @%s, ", numberTypeLLVMReprs[lv.num_info.number_type],
            REFSTRING(entry->type.value.number.pointer_depth), symbol_name);
    fprintf(D_LLVM_FILE, "%s%s %%%lld" NEWLINE, numberTypeLLVMReprs[lv.num_info.number_type],
            REFSTRING(lv.num_info.pointer_depth), free_reg);

    return lv;
}

/**
 * @brief Generate a dereference (load) statement
 * 
 * @param reg           Register to dereference
 * @return LLVMValue    LLVMValue containing loaded value
 */
LLVMValue llvm_dereference(LLVMValue reg)
{
    LLVMValue out = LLVMVALUE_VIRTUAL_REGISTER_POINTER(get_next_local_virtual_register(),
                                                       reg.num_info.number_type,
                                                       reg.num_info.pointer_depth - 1);

    print_function_annotation("llvm_dereference");

    fprintf(D_LLVM_FILE, TAB "%%%lld = load %s%s, ", out.value.virtual_register_index,
            numberTypeLLVMReprs[out.num_info.number_type], REFSTRING(out.num_info.pointer_depth));
    fprintf(D_LLVM_FILE, "%s%s %%%lld" NEWLINE, numberTypeLLVMReprs[reg.num_info.number_type],
            REFSTRING(reg.num_info.pointer_depth), reg.value.virtual_register_index);

    return out;
}

/**
 * @brief Generates code to store a value into a dereferenced value
 * 
 * @param destination   Destination register or variable to store into
 * @param value         Value to store
 */
void llvm_store_dereference(LLVMValue destination, LLVMValue value)
{
    LLVMValue* loaded_registers = llvm_ensure_registers_loaded(1, (LLVMValue[]){destination},
                                                               value.num_info.pointer_depth + 1);
    if (loaded_registers) {
        destination = loaded_registers[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers", "llvm_store_dereference");
        free(loaded_registers);
        loaded_registers = NULL;
    }

    if (value.value.constant == 5) {
        PRINT_LLVMVALUE(destination);
        PRINT_LLVMVALUE(value);
        printf("--------------------\n");
    }

    // loaded_registers = llvm_ensure_registers_loaded(
    //     1, (LLVMValue[]){value},
    //     destination.num_info.pointer_depth - 1 + (strlen(destination.just_loaded) == 0 ? 0 : 1));
    // if (loaded_registers) {
    //     value = loaded_registers[0];
    //     purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers (1)", "llvm_store_dereference");
    //     free(loaded_registers);
    //     loaded_registers = NULL;
    // }

    print_function_annotation("llvm_store_dereference");

    if (strlen(destination.just_loaded) == 0 ||
        destination.num_info.pointer_depth == value.num_info.pointer_depth + 1) {
        fprintf(D_LLVM_FILE, TAB "store %s%s %s, ", numberTypeLLVMReprs[value.num_info.number_type],
                REFSTRING(value.num_info.pointer_depth), LLVM_REPR_NOTYPE(value));
        fprintf(D_LLVM_FILE, "%s%s %s" NEWLINE,
                numberTypeLLVMReprs[destination.num_info.number_type],
                REFSTRING(destination.num_info.pointer_depth), LLVM_REPR_NOTYPE(destination));
    } else {
        fprintf(D_LLVM_FILE, TAB "store %s%s %s, ", numberTypeLLVMReprs[value.num_info.number_type],
                REFSTRING(value.num_info.pointer_depth), LLVM_REPR_NOTYPE(value));
        fprintf(D_LLVM_FILE, "%s%s* @%s" NEWLINE,
                numberTypeLLVMReprs[destination.num_info.number_type],
                REFSTRING(destination.num_info.pointer_depth), destination.just_loaded);
    }
}

void llvm_store_local(char* symbol_name, LLVMValue val)
{
    SymbolTableEntry* ste = STS_FIND(symbol_name);
    if (!ste) {
        fatal(RC_COMPILER_ERROR, "Tried to store into NULL SymbolTableEntry in llvm_store_local");
    }

    ste->latest_llvmvalue = val;
}

void llvm_store_local_llvmvalue(LLVMValue destination, LLVMValue val)
{
    LLVMValue* loaded_registers =
        llvm_ensure_registers_loaded(1, (LLVMValue[]){val}, destination.num_info.pointer_depth - 1);
    if (loaded_registers) {
        destination = loaded_registers[0];
        purple_log(LOG_DEBUG, "Freeing %s in %s", "loaded_registers", "llvm_store_local");
        free(loaded_registers);
        loaded_registers = NULL;
    }
    val = llvm_int_resize(val, destination.num_info.number_type);

    print_function_annotation("llvm_store_local_llvmvalue");
    char** val_dest_reprs = (char*[]){llvmvalue_repr(val), llvmvalue_repr(destination)};
    fprintf(D_LLVM_FILE, TAB "store %s, %s" NEWLINE, val_dest_reprs[0], val_dest_reprs[1]);

    purple_log(LOG_DEBUG, "Freeing val_dest_reprs in llvm_store_local_llvmvalue");
    free(val_dest_reprs[1]);
    free(val_dest_reprs[0]);
}
