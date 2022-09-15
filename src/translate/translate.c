/**
 * @file translate.c
 * @author Charles Averill
 * @brief Functions for LLVM-IR translation
 * @date 10-Sep-2022
 */

#include "translate/translate.h"
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

    D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER = 1;

    initialize_virtual_registers();

    loadedRegistersHead = NULL;
}

void initialize_virtual_registers() { freeVirtualRegistersHead = NULL; }

void add_free_virtual_register(type_register register_index)
{
    LLVMStackEntryNode* temp = (LLVMStackEntryNode*)malloc(sizeof(LLVMStackEntryNode));
    temp->reg = register_index;
    temp->next = NULL;

    if (freeVirtualRegistersHead == NULL) {
        freeVirtualRegistersHead = temp;
    } else {
        temp->next = freeVirtualRegistersHead;
        freeVirtualRegistersHead = temp;
    }
}

type_register get_free_virtual_register(void)
{
    LLVMStackEntryNode* temp;

    if (freeVirtualRegistersHead == NULL) {
        fatal(RC_COMPILER_ERROR, "Tried to get a free virtual register when there are none left");
    }

    type_register out = freeVirtualRegistersHead->reg;
    temp = freeVirtualRegistersHead->next;
    free(freeVirtualRegistersHead);
    freeVirtualRegistersHead = temp;

    return out;
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
    } else {
        LLVMStackEntryNode* current = (LLVMStackEntryNode*)malloc(sizeof(LLVMStackEntryNode));
        current->reg = get_next_local_virtual_register();
        add_free_virtual_register(current->reg);
        current->type = token_type_to_number_type(root->ttype);
        current->align_bytes = numberTypeByteSizes[current->type];
        current->next = NULL;
        return current;
    }
}

/**
 * @brief Generates LLVM-IR from a given AST
 * @param n The AST Node from which LLVM will be generated
 * @return LLVMValue LLVMValue struct containing information about what code this AST Node generated
*/
LLVMValue ast_to_llvm(ASTNode* n)
{
    type_register virtual_registers[2] = {0, 0};
    LLVMValue temp_values[2];

    // Make sure we aren't trying to generate from a null node
    if (!n) {
        return LLVMVALUE_NULL;
    }

    // Generate code for left and right subtrees
    temp_values[0] = ast_to_llvm(n->left);
    temp_values[1] = ast_to_llvm(n->right);

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
    } else if (TOKENTYPE_IS_TERMINAL(n->ttype)) {
        switch (n->ttype) {
        case T_INTEGER_LITERAL:
            return llvm_store_constant(NUMBER_INT32(n->value));
        default:
            break;
        }
    } else {
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Unknown operator \"%s\"", tokenStrings[n->ttype]);
    }
}

/**
 * @brief Free memory used by LLVMStackEntryNode linked lists
 * 
 * @param head Head of list to free
 */
void free_llvm_stack_entry_node_list(LLVMStackEntryNode* head)
{
    LLVMStackEntryNode* current = head;
    LLVMStackEntryNode* prev;
    while (current) {
        prev = current;
        current = current->next;
        free(prev);
    }
    head = NULL;
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
