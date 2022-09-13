/**
 * @file llvm.c
 * @author Charles Averill
 * @brief LLVM-IR emission
 * @date 12-Sep-2022
 */

#include "translate/llvm.h"
#include "data.h"
#include "utils/formatting.h"
#include "utils/logging.h"

/**
 * @brief Generated program's preamble
 */
void llvm_preamble()
{
    fprintf(D_LLVM_FILE, "; ModuleID = '%s'" NEWLINE, D_INPUT_FN);
    fprintf(D_LLVM_FILE,
            "target datalayout = "
            "\"e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128\"" NEWLINE);
    fprintf(D_LLVM_FILE, "target triple = \"%s\"" NEWLINE NEWLINE, "x86_64-pc-linux-gnu");
    fprintf(D_LLVM_FILE, "; Function Attrs: noinline nounwind optnone uwtable" NEWLINE);
    fprintf(D_LLVM_FILE, "define dso_local i32 @main() #0 {" NEWLINE);
}

/**
 * @brief Generated program's postamble
 */
void llvm_postamble()
{
    fprintf(D_LLVM_FILE, "}" NEWLINE NEWLINE);
    fprintf(
        D_LLVM_FILE,
        "attributes #0 = {noinline nounwind optnone uwtable \"frame-pointer\"=\"all\" "
        "\"min-legal-vector-width\"=\"0\" \"no-trapping-math\"=\"true\" "
        "\"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86_64\" "
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

/**
 * @brief Allocate space on stack for variables
 * 
 * @param stack_entries LLVMStackEntryNode pointers holding stack allocation information
 */
void llvm_stack_allocation(LLVMStackEntryNode* stack_entries)
{
    LLVMStackEntryNode* current = stack_entries;

    while (current) {
        fprintf(D_LLVM_FILE, TAB "%%%llu = alloca %s, align %d" NEWLINE, current->reg,
                numberTypeLLVMReprs[current->type], current->align_bytes);
        current = current->next;
    }
}

static int llvm_add(int left_virtual_register, int right_virtual_register)
{
    fprintf(D_LLVM_FILE, TAB "%%%llu = add nsw i32 %%%d, %%%d" NEWLINE,
            get_next_local_virtual_register(), left_virtual_register, right_virtual_register);
    return D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER;
}

static int llvm_subtract(int left_virtual_register, int right_virtual_register)
{
    fprintf(D_LLVM_FILE, TAB "%%%llu = sub nsw i32 %%%d, %%%d" NEWLINE,
            get_next_local_virtual_register(), left_virtual_register, right_virtual_register);
    return D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER;
}

static int llvm_multiply(int left_virtual_register, int right_virtual_register)
{
    fprintf(D_LLVM_FILE, TAB "%%%llu = mul nsw i32 %%%d, %%%d" NEWLINE,
            get_next_local_virtual_register(), left_virtual_register, right_virtual_register);
    return D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER;
}

static int llvm_divide(int left_virtual_register, int right_virtual_register)
{
    fprintf(D_LLVM_FILE, TAB "%%%llu = div nsw i32 %%%d, %%%d" NEWLINE,
            get_next_local_virtual_register(), left_virtual_register, right_virtual_register);
    return D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER;
}

/**
 * @brief Generates LLVM-IR for various binary arithmetic expressions
 * 
 * @param operation Operation to perform
 * @param left_virtual_register Operand left of operation
 * @param right_virtual_register Operand right of operation
 * @return int Number of virtual register in which the result is stored
 */
int llvm_binary_arithmetic(TokenType operation, int left_virtual_register,
                           int right_virtual_register)
{
    switch (operation) {
    case T_PLUS:
        return llvm_add(left_virtual_register, right_virtual_register);
    case T_MINUS:
        return llvm_subtract(left_virtual_register, right_virtual_register);
    case T_STAR:
        return llvm_multiply(left_virtual_register, right_virtual_register);
    case T_SLASH:
        return llvm_divide(left_virtual_register, right_virtual_register);
    default:
        fatal(RC_COMPILER_ERROR,
              "llvm_binary_arithmetic receieved non-binary-arithmetic operator \"%s\"",
              tokenStrings[operation]);
    }
}

/**
 * @brief Load a constant number value into a register
 * 
 * @param value Number struct containing information about the constant
 * @return int Register number value is held in
 */
int llvm_load_constant(Number value)
{
    fprintf(D_LLVM_FILE, TAB "store %s ", numberTypeLLVMReprs[value.type]);
    fprintf(D_LLVM_FILE, numberTypeFormatStrings[value.type], value.value);
    fprintf(D_LLVM_FILE, ", %s* %%%llu, align %d" NEWLINE, numberTypeLLVMReprs[value.type],
            D_FREE_REGISTER_COUNT--, numberTypeByteSizes[value.type]);
}

unsigned long long int get_next_local_virtual_register(void)
{
    return D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER++;
}
