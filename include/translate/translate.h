/**
 * @file translate.h
 * @author Charles Averill
 * @brief Function headers and definitions for translation of an AST into LLVM-IR
 * @date 10-Sep-2022
 */

#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "parse.h"
#include "translate/llvm.h"
#include "tree.h"

LLVMStackEntryNode* determine_binary_expression_stack_allocation(ASTNode* root);

LLVMValue ast_to_llvm(ASTNode* n, type_register register_number);
void generate_llvm(void);

#endif /* TRANSLATE_H */
