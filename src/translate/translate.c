/**
 * @file translate.c
 * @author Charles Averill
 * @brief Functions for LLVM-IR translation
 * @date 10-Sep-2022
 */

#include "translate/translate.h"
#include "data.h"
#include "utils/logging.h"

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

    D_CURRENT_FUNCTION_PREAMBLE_PRINTED = false;
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
    } else if (TOKENTYPE_IS_LITERAL(root->ttype)) {
        LLVMStackEntryNode* current = (LLVMStackEntryNode*)malloc(sizeof(LLVMStackEntryNode));

        current->reg = get_next_local_virtual_register();
        prepend_stack_entry_linked_list(&freeVirtualRegistersHead, current->reg);

        current->type = root->tree_type.type;
        current->align_bytes = numberTypeByteSizes[current->type];
        current->next = NULL;

        return current;
    } else if (root->ttype == T_IDENTIFIER) {
        SymbolTableEntry* symbol =
            find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, root->value.symbol_name);
        if (symbol == NULL) {
            fatal(RC_COMPILER_ERROR,
                  "Failed to find symbol in determine_binary_expression_stack_allocation");
        }

        LLVMStackEntryNode* current = (LLVMStackEntryNode*)malloc(sizeof(LLVMStackEntryNode));

        current->reg = get_next_local_virtual_register();
        prepend_stack_entry_linked_list(&freeVirtualRegistersHead, current->reg);

        current->type = symbol->type.value.number.type;
        if (current->type == -1) {
            fatal(RC_COMPILER_ERROR, "Symbol number type is ill-formed");
        }

        current->align_bytes = numberTypeByteSizes[symbol->type.value.number.type];
        current->next = NULL;

        return current;
    }

    return NULL;
}

/**
 * @brief Generate LLVM-IR for an if statement AST
 * 
 * jump false_label if !condition
 * body()
 * jump end_label
 * false_label:
 * falsebody()
 * end_label:
 * 
 * @param n Root AST node
 * @return LLVMValue LLVMVALUE_NULL
 */
static LLVMValue if_ast_to_llvm(ASTNode* n)
{
    LLVMValue false_label, end_label;

    false_label = get_next_label();
    if (n->right) {
        end_label = get_next_label();
    }

    ast_to_llvm(n->left, false_label, n->ttype);
    ast_to_llvm(n->mid, LLVMVALUE_NULL, n->ttype);

    if (n->right) {
        llvm_jump(end_label);
    } else {
        llvm_jump(false_label);
    }

    llvm_label(false_label);

    if (n->right) {
        ast_to_llvm(n->right, LLVMVALUE_NULL, n->ttype);

        llvm_jump(end_label);
        llvm_label(end_label);
    }

    return LLVMVALUE_NULL;
}

/**
 * @brief Generate LLVM-IR for a while-else statement AST
 * 
 * condition_label:
 * jump else_label if !condition
 * body()
 * jump condition_label
 * else_label:
 * elsebody()
 * 
 * @param n Root AST node
 * @return LLVMValue LLVMVALUE_NULL
 */
static LLVMValue while_else_ast_to_llvm(ASTNode* n)
{
    LLVMValue condition_label, else_label;

    condition_label = get_next_label();
    else_label = get_next_label();

    llvm_jump(condition_label);
    llvm_label(condition_label);

    ast_to_llvm(n->left, else_label, n->ttype);
    ast_to_llvm(n->mid, LLVMVALUE_NULL, n->ttype);

    if (n->right) {
        switch (n->right->ttype) {
        case T_AST_GLUE:
            // For loop
            ast_to_llvm(n->right->left, LLVMVALUE_NULL, n->ttype);
            llvm_jump(condition_label);
            llvm_label(else_label);
            ast_to_llvm(n->right->right, LLVMVALUE_NULL, n->ttype);
            break;
        default:
            // Standard while loop
            llvm_jump(condition_label);
            llvm_label(else_label);
            ast_to_llvm(n->right, LLVMVALUE_NULL, n->ttype);
        }
    } else {
        llvm_jump(condition_label);
        llvm_label(else_label);
    }

    return LLVMVALUE_NULL;
}

/**
 * @brief Generate LLVM-IR for a print statement
 * 
 * @param n Root AST node
 * @return LLVMValue LLVMVALUE_NULL
 */
static LLVMValue print_ast_to_llvm(ASTNode* root, LLVMValue virtual_register)
{
    if (root->left->tree_type.type == NT_INT1) {
        llvm_print_bool(virtual_register);
    } else {
        TokenType print_type = root->left->ttype;

        if (print_type == T_IDENTIFIER) {
            print_type = number_to_token_type((Number){.type = root->left->largest_number_type});
        }

        if (print_type == T_FUNCTION_CALL) {
            print_type =
                find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, root->left->value.symbol_name)
                    ->type.value.function.return_type;
        }

        if (TOKENTYPE_IS_BINARY_ARITHMETIC(print_type)) {
            print_type = number_to_token_type((Number){.type = root->left->largest_number_type});
        }

        if (print_type == T_BYTE_LITERAL || root->left->is_char_arithmetic) {
            print_type = T_CHAR_LITERAL;
        }

        purple_log(LOG_DEBUG, "Printing int with print_type %d", print_type);
        llvm_print_int(virtual_register);
    }

    return LLVMVALUE_NULL;
}

/**
 * @brief Generates LLVM-IR from a given AST
 * 
 * @param n The AST Node from which LLVM will be generated
 * @param llvm_value LLVMValue storing register or label information
 * @param parent_operation TokenType of parent of n
 * @return LLVMValue LLVMValue struct containing information about what code this AST Node generated
*/
LLVMValue ast_to_llvm(ASTNode* root, LLVMValue llvm_value, TokenType parent_operation)
{
    LLVMValue virtual_registers[2];
    LLVMValue temp_values[2];
    LLVMValue out;
    SymbolTableEntry* symbol;

    // Make sure we aren't trying to generate from a null node
    if (!root) {
        return LLVMVALUE_NULL;
    }

    // Special kinds of TokenTypes that shouldn't have their left and right branches generated in the standard manner
    switch (root->ttype) {
    case T_IF:
        return if_ast_to_llvm(root);
    case T_WHILE:
        return while_else_ast_to_llvm(root);
    case T_AST_GLUE:
        ast_to_llvm(root->left, LLVMVALUE_NULL, root->ttype);
        ast_to_llvm(root->mid, LLVMVALUE_NULL, root->ttype);
        ast_to_llvm(root->right, LLVMVALUE_NULL, root->ttype);
        return LLVMVALUE_NULL;
    case T_FUNCTION_DECLARATION:
        llvm_function_preamble(root->value.symbol_name);
        ast_to_llvm(root->left, LLVMVALUE_NULL, root->ttype);
        if (!D_CURRENT_FUNCTION_HAS_RETURNED) {
            llvm_return(LLVMVALUE_CONSTANT(0), root->value.symbol_name);
        }
        llvm_function_postamble();
        return LLVMVALUE_NULL;
    default:
        break;
    }

    // Generate code for left and right subtrees
    temp_values[0] = ast_to_llvm(root->left, LLVMVALUE_NULL, root->ttype);
    temp_values[1] = ast_to_llvm(root->right, temp_values[0], root->ttype);

    // Process values from left and right subtrees
    for (int i = 0; i < 2; i++) {
        switch (temp_values[i].value_type) {
        case LLVMVALUETYPE_VIRTUAL_REGISTER:
            virtual_registers[i] = temp_values[i];
            break;
        case LLVMVALUETYPE_NONE:
        default:
            break;
        }
    }

    LLVMValue left_vr = virtual_registers[0];
    LLVMValue right_vr = virtual_registers[1];

    if (TOKENTYPE_IS_BINARY_ARITHMETIC(root->ttype)) {
        return llvm_binary_arithmetic(root->ttype, left_vr, right_vr);
    } else if (TOKENTYPE_IS_COMPARATOR(root->ttype)) {
        if (parent_operation == T_IF || parent_operation == T_WHILE) {
            if (llvm_value.value_type != LLVMVALUETYPE_LABEL) {
                fatal(RC_COMPILER_ERROR, "Tried to generate an if branch but received an LLVMValue "
                                         "without a label index");
            }

            return llvm_compare_jump(root->ttype, left_vr, right_vr, llvm_value);
        } else {
            return llvm_compare(root->ttype, left_vr, right_vr);
        }
    } else if (TOKENTYPE_IS_LOGICAL_OPERATOR(root->ttype)) {
        if (root->left->tree_type.type != NT_INT1 ||
            root->left->tree_type.type != root->right->tree_type.type) {
            syntax_error(root->filename, root->line_number, root->char_number,
                         "Cannot perform logical \"%s\" comparison on types %s and %s",
                         tokenStrings[root->ttype], numberTypeLLVMReprs[root->left->tree_type.type],
                         numberTypeLLVMReprs[root->right->tree_type.type]);
        }

        if (parent_operation == T_IF || parent_operation == T_WHILE) {
            if (llvm_value.value_type != LLVMVALUETYPE_LABEL) {
                fatal(RC_COMPILER_ERROR, "Tried to generate an if branch but received an LLVMValue "
                                         "without a label index");
            }

            return llvm_compare_jump(root->ttype, left_vr, right_vr, llvm_value);
        } else {
            return llvm_compare(root->ttype, left_vr, right_vr);
        }
    } else if (TOKENTYPE_IS_LITERAL(root->ttype)) {
        // Allocate stack space
        purple_log(LOG_DEBUG, "Determining stack space");
        LLVMStackEntryNode* stack_entries = determine_binary_expression_stack_allocation(root);
        purple_log(LOG_DEBUG, "Allocating stack space");
        if (llvm_stack_allocation(stack_entries)) {
            purple_log(LOG_DEBUG, "Freeing stack space entries");
            free_llvm_stack_entry_node_list(stack_entries);
            stack_entries = NULL;
        }

        NumberType store_type;
        if ((store_type = token_type_to_number_type(root->ttype)) != -1) {
            out = llvm_store_constant(NUMBER_FROM_TYPE_VAL(store_type, root->value.number_value));
        } else {
            fatal(RC_COMPILER_ERROR, "Failed to match TokenType \"%s\" to NumberType",
                  tokenStrings[root->ttype]);
        }

        return out;
    } else {
        switch (root->ttype) {
        case T_IDENTIFIER:
            return llvm_load_global_variable(root->value.symbol_name);
        case T_LVALUE_IDENTIFIER:;
            symbol = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, root->value.symbol_name);
            if (symbol == NULL) {
                fatal(RC_COMPILER_ERROR, "Failed to find symbol \"%s\" in Global Symbol Table",
                      root->value.symbol_name);
            }

            llvm_store_global_variable(root->value.symbol_name, llvm_value);
            return LLVMVALUE_VIRTUAL_REGISTER(llvm_value.value.virtual_register_index,
                                              symbol->type.value.number.type);
        case T_ASSIGN:
            return LLVMVALUE_VIRTUAL_REGISTER(llvm_value.value.virtual_register_index, NT_INT1);
        case T_PRINT:
            return print_ast_to_llvm(root, left_vr);
        case T_FUNCTION_CALL:;
            symbol = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, root->value.symbol_name);
            if (symbol == NULL) {
                fatal(RC_COMPILER_ERROR, "Failed to find symbol \"%s\" in Global Symbol Table",
                      root->value.symbol_name);
            }
            return llvm_call_function(left_vr, root->value.symbol_name);
        case T_AMPERSAND:
            return llvm_get_address(root->value.symbol_name);
        case T_DEREFERENCE:
            return llvm_dereference(left_vr);
        case T_RETURN:
            symbol = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, root->value.symbol_name);
            if (symbol == NULL) {
                symbol = find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, D_CURRENT_FUNCTION_BUFFER);
                if (symbol == NULL) {
                    symbol =
                        find_symbol_table_entry(D_GLOBAL_SYMBOL_TABLE, D_CURRENT_FUNCTION_BUFFER);
                    fatal(RC_COMPILER_ERROR, "Failed to find symbol \"%s\" in Global Symbol Table",
                          D_CURRENT_FUNCTION_BUFFER);
                }
            }

            llvm_return(

                left_vr, root->value.symbol_name);
            return LLVMVALUE_NULL;
        default:
            fatal(RC_COMPILER_ERROR, "Unknown operator \"%s\"", tokenStrings[root->ttype]);
        }
    }

    return LLVMVALUE_NULL;
}

/**
 * @brief Wrapper function for generating LLVM
 */
void generate_llvm(void)
{
    purple_log(LOG_DEBUG, "Beginning translation");

    translate_init();

    llvm_preamble();

    while (D_GLOBAL_TOKEN.type != T_EOF) {
        D_CURRENT_FUNCTION_HAS_RETURNED = false;
        ASTNode* root = function_declaration();
        ast_to_llvm(root, LLVMVALUE_NULL, root->ttype);
        D_CURRENT_FUNCTION_PREAMBLE_PRINTED = false;

        D_LLVM_LOCAL_VIRTUAL_REGISTER_NUMBER = 1;

        free_llvm_stack_entry_node_list(freeVirtualRegistersHead);
        freeVirtualRegistersHead = NULL;
    }

    llvm_postamble();

    purple_log(LOG_DEBUG, "LLVM written to %s", D_LLVM_FN);
}
