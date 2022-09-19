/**
 * @file translate.c
 * @author Charles Averill
 * @brief Functions for LLVM-IR translation
 * @date 10-Sep-2022
 */

#include "translate/translate.h"
#include "data.h"
#include "utils/logging.h"

LLVMStackEntryNode* loadedRegistersHead = NULL;
LLVMStackEntryNode* freeVirtualRegistersHead = NULL;

/**
 * @brief Initialize any values required for LLVM translation
 */
static void translate_init(void)
{
    D_LLVM_FN = args->filenames[1];
    D_LLVM_FILE = fopen(args->filenames[1], "w");
    if (D_LLVM_FILE == NULL) {
        fatal(RC_FILE_ERROR, "Could not open %s for writing LLVM", args->filenames[1]);
    }

    D_LLVM_GLOBALS_FN = "globals.ll";
    D_LLVM_GLOBALS_FILE = fopen(D_LLVM_GLOBALS_FN, "w");
    if (D_LLVM_GLOBALS_FILE == NULL) {
        fatal(RC_FILE_ERROR, "Could not open %s for writing LLVM global variables",
              D_LLVM_GLOBALS_FN);
    }

    D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER = 1;

    initialize_stack_entry_linked_list(&freeVirtualRegistersHead);
    initialize_stack_entry_linked_list(&loadedRegistersHead);
}

/**
 * @brief Perform a DFS on an AST to determine the stack allocation needed for a binary expression
 * 
 * @param root Root of AST to find stack allocation for
 * @return LLVMStackEntryNode* Pointer to front of the stack allocation linked list
 */
LLVMStackEntryNode* determine_binary_expression_stack_allocation(ASTNode* root)
{
    LLVMStackEntryNode* temp_left;
    LLVMStackEntryNode* temp_right;
    LLVMStackEntryNode* curr;

    if (root == NULL) {
        return NULL;
    }

    if (root->left || root->right) {
        if (root->left) {
            temp_left = determine_binary_expression_stack_allocation(root->left);
        }
        if (root->right) {
            temp_right = determine_binary_expression_stack_allocation(root->right);
        }

        if (temp_left) {
            curr = temp_left;
            while (curr->next) {
                curr = curr->next;
            }
            curr->next = temp_right;
        } else {
            temp_left = temp_right;
        }

        return temp_left;
    } else if (root->ttype == T_INTEGER_LITERAL) {
        LLVMStackEntryNode* current = (LLVMStackEntryNode*)malloc(sizeof(LLVMStackEntryNode));
        current->reg = get_next_local_virtual_register();
        prepend_stack_entry_linked_list(&freeVirtualRegistersHead, current->reg);
        current->type = token_type_to_number_type(root->ttype);
        current->align_bytes = numberTypeByteSizes[current->type];
        current->next = NULL;
        return current;
    } else if (root->ttype == T_IDENTIFIER) {
        LLVMStackEntryNode* current = (LLVMStackEntryNode*)malloc(sizeof(LLVMStackEntryNode));
        current->reg = get_next_local_virtual_register();
        prepend_stack_entry_linked_list(&freeVirtualRegistersHead, current->reg);
        current->type = token_type_to_number_type(T_INTEGER_LITERAL);
        current->align_bytes = numberTypeByteSizes[current->type];
        current->next = NULL;
        return current;
    }
}

/**
 * @brief Generates LLVM-IR from a given AST
 * 
 * @param n The AST Node from which LLVM will be generated
 * @param register_number Register number of RValues for storing into LValues
 * @return LLVMValue LLVMValue struct containing information about what code this AST Node generated
*/
LLVMValue ast_to_llvm(ASTNode* n, type_register register_number)
{
    type_register virtual_registers[2] = {0, 0};
    LLVMValue temp_values[2];

    // Make sure we aren't trying to generate from a null node
    if (!n) {
        return LLVMVALUE_NULL;
    }

    // Generate code for left and right subtrees
    temp_values[0] = ast_to_llvm(
        n->left,
        0); // TODO: Here, register_number should be something like -1 to show that it is not actually representing a valid register. Shouldn't matter anyways but it's iffy and I don't like it
    temp_values[1] = ast_to_llvm(n->right, temp_values[0].value.virtual_register_index);

    // Process values from left and right subtrees
    for (int i = 0; i < 2; i++) {
        switch (temp_values[i].value_type) {
        case LLVMVALUETYPE_VIRTUAL_REGISTER:
            virtual_registers[i] = temp_values[i].value.virtual_register_index;
            break;
        case LLVMVALUETYPE_NONE:
        default:
            break;
        }
    }

    type_register left_vr = virtual_registers[0];
    type_register right_vr = virtual_registers[1];

    if (TOKENTYPE_IS_BINARY_ARITHMETIC(n->ttype)) {
        type_register* loaded_registers =
            llvm_ensure_registers_loaded(2, (type_register[]){left_vr, right_vr});
        if (loaded_registers != NULL) {
            left_vr = loaded_registers[0];
            right_vr = loaded_registers[1];
            free(loaded_registers);
        }

        return llvm_binary_arithmetic(n->ttype, LLVMVALUE_VIRTUAL_REGISTER(left_vr),
                                      LLVMVALUE_VIRTUAL_REGISTER(right_vr));
    } else {
        switch (n->ttype) {
        case T_INTEGER_LITERAL:
            return llvm_store_constant(NUMBER_INT32(n->value.int_value));
        case T_IDENTIFIER:
            return llvm_load_global_variable(
                D_GLOBAL_SYMBOL_TABLE->buckets[n->value.global_symbol_table_index]->symbol_name);
        case T_LVALUE_IDENTIFIER:
            llvm_store_global_variable(
                D_GLOBAL_SYMBOL_TABLE->buckets[n->value.global_symbol_table_index]->symbol_name,
                register_number);
            return LLVMVALUE_VIRTUAL_REGISTER(register_number);
        case T_ASSIGN:
            return LLVMVALUE_VIRTUAL_REGISTER(register_number);
        default:
            syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Unknown operator \"%s\"",
                         tokenStrings[n->ttype]);
        }
    }
}

/**
 * @brief Wrapper function for generating LLVM
 */
void generate_llvm(void)
{
    purple_log(LOG_DEBUG, "Beginning translation");

    translate_init();

    llvm_preamble();

    parse_statements();

    llvm_postamble();

    free_llvm_stack_entry_node_list(loadedRegistersHead);
    free_llvm_stack_entry_node_list(freeVirtualRegistersHead);

    purple_log(LOG_DEBUG, "LLVM written to %s", D_LLVM_FN);
}
