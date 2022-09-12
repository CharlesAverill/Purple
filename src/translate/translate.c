/**
 * @file translate.c
 * @author Charles Averill
 * @brief Functions for LLVM-IR translation
 * @date 10-Sep-2022
 */

#include "translate/translate.h"
#include "translate/llvm.h"
#include "types/number.h"
#include "utils/logging.h"

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

    D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER = 0;
}

/**
 * @brief Exit code for the translator including freeing registers and closing the ASM file
 */
static void exit_translator(void) { fclose(D_LLVM_FILE); }

/**
 * @brief Generates LLVM-IR from a given AST
 * @param n The AST Node from which LLVM will be generated
 * @return LLVMValue LLVMValue struct containing information about what code this AST Node generated
*/
LLVMValue ast_to_llvm(ASTNode* n)
{
    int virtual_registers[2] = {-1, -1};
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

    int left_vr = virtual_registers[0];
    int right_vr = virtual_registers[1];

    if (TOKENTYPE_IS_BINARY_ARITHMETIC(n->ttype)) {
        return LLVMVALUE_VIRTUAL_REGISTER(llvm_binary_arithmetic(n->ttype, left_vr, right_vr));
    } else if(TOKENTYPE_IS_TERMINAL(n->ttype)) {
        switch(n->ttype) {
            case T_INTEGER_LITERAL:
                return LLVMVALUE_VIRTUAL_REGISTER(llvm_load_constant(NUMBER_INT32(n->value)));
            default:
                break;
        }
    } else {
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Unknown operator \"%s\"", tokenStrings[n->ttype]);
    }
}

/**
 * @brief Wrapper function for generating LLVM
 */
void generate_llvm(ASTNode* root)
{
    translate_init();

    // Parse everything into a single AST
    ast_to_llvm(root);

    exit_translator();

    purple_log(LOG_DEBUG, "LLVM written to %s", args->filenames[1]);
}
