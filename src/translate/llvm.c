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
; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, i8** noundef %1) #0 {
...
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 14.0.0-1ubuntu1"}
 */
void llvm_preamble() {
    fprintf(D_LLVM_FILE, "; ModuleID = '%s'" NEWLINE, D_INPUT_FN);
    fprintf(D_LLVM_FILE, "target datalayout = \"e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128\"" NEWLINE);
    fprintf(D_LLVM_FILE, "target triple = \"%s\"" NEWLINE, "x86_64-pc-linux-gnu");
}

void llvm_postamble() {

}

static int llvm_add(int left_virtual_register, int right_virtual_register) {
    increment_local_virtual_register_number();
    fprintf(D_LLVM_FILE, TAB "%%%d = add nsw i32 %%%d, %%%d" NEWLINE, D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER, left_virtual_register, right_virtual_register);
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
    case T_STAR:
    case T_SLASH:
        break;
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
int llvm_load_number(Number value) {

}

void increment_local_virtual_register_number(void) {
    D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER += 1;
}
