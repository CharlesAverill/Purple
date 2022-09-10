/**
 * @file purple.c
 * @author Charles Averill
 * @brief Compiler entrypoint and setup
 * @date 08-Sep-2022
 */

#include <stdio.h>

// Define, then undef extern_ to transfer ownership to purple.c
#define extern_
#include "data.h"
#undef extern_

#include "arguments.h"
#include "errors_warnings.h"
#include "parse.h"
#include "scan.h"

static void init(int argc, char* argv[]);
void shutdown(void);

/**
 * @brief Parse compiler arguments, open input file, and allocate memory 
 * 
 * @param argc 
 * @param argv 
 */
static void init(int argc, char* argv[])
{
    // Global data
    D_LINE_NUMBER = 1;
    D_PUT_BACK = '\n';

    // Argument parsing
    args = malloc(sizeof(purple_args));
    if (args == NULL) {
        fatal(RC_MEMORY_ERROR, "Unable to allocate memory for command line arguments");
    }

    parse_args(args, argc, argv);

    D_INPUT_FN = args->filenames[0];
    D_INPUT_FILE = fopen(D_INPUT_FN, "r");
    if (D_INPUT_FILE == NULL) {
        fatal(RC_FILE_ERROR, "Unable to open %s: %s\n", D_INPUT_FN, strerror(errno));
    }

    // Global Token
    scan(&D_GLOBAL_TOKEN);
}

/**
 * @brief Tie up any loose ends that may have arisen
 */
void shutdown(void)
{
    if (D_INPUT_FILE) {
        fclose(D_INPUT_FILE);
    }
}

int interpretAST(struct ASTNode* n)
{
    int leftval, rightval;

    // Get the left and right sub-tree values
    if (n->left)
        leftval = interpretAST(n->left);
    if (n->right)
        rightval = interpretAST(n->right);

    // Debug: Print what we are about to do
    if (n->ttype == T_INTEGER_LITERAL)
        printf("int %d\n", n->value);
    else
        printf("%d %s %d\n", leftval, token_strings[n->ttype], rightval);

    switch (n->ttype) {
    case T_PLUS:
        return (leftval + rightval);
    case T_MINUS:
        return (leftval - rightval);
    case T_STAR:
        return (leftval * rightval);
    case T_SLASH:
        return (leftval / rightval);
    case T_INTEGER_LITERAL:
        return (n->value);
    default:
        fatal(RC_ERROR, "Unknown AST operator %d\n", n->ttype);
    }
}

/**
 * @brief Compiler entrypoint
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return int Compiler return code
 */
int main(int argc, char* argv[])
{
    init(argc, argv);

    struct ASTNode* n;
    n = parse_binary_expression(0);
    printf("%d\n", interpretAST(n));

    shutdown();

    return 0;
}