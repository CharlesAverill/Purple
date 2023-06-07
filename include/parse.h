/**
 * @file parse.h
 * @author Charles Averill
 * @brief Function headers for parsing and operator precedence values
 * @date 09-Sep-2022
 * 
 * @subsection operatorPrecedence Operator Precedence
 * This table outlines Purple's operator precedence values. 
 * A higher precedence value is computed before a lower precedence value.
 * | Precedence | Operator | Description | Associativity |
 * | ---------- | -------- | ----------- | ------------- |
 * | 16         | `++ --` <br> `()` <br> `[]` <br> `.` <br> `->` <br> <i>`(type){list}`</i> </br> | Postfix increment/decrement <br> Function call <br> Array subscript <br> Structure and Union member access <br> Structure and union pointer member access <br> Compound literal | Left-to-Right |
 * | 15         | `**` <br> `++ --` <br> `-` <br> `! ~` <br> <i>`(type)`</i> <br> `*` <br> `&` <br> `bytesize` | Exponent <br> Prefix increment/decrement <br> Unary negative <br> Logical NOT, bitwise NOT <br> Cast <br> Dereference <br> Address-of <br> Size in bytes | Right-to-Left |
 * | 14         | `* / %`  | Multiplication, division, modulus | Left-to-Right |
 * | 13         | `+ -` | Addition, Subtraction | |
 * | 12         | `<< >>` | Bitwise shifts | |
 * | 11         | `< <=` <br> `> >=` | "Less than", "Less than or equal to" relational operators <br> "Greater than", "Greater than or equal to" relational operators | |
 * | 10         | `== !=` | "Is equal", "Is not equal" relational operators | |
 * |  9         | `&` | Bitwise AND | |
 * |  8         | `^` | Bitwise XOR | |
 * |  7         | \| | Bitwise OR | |
 * |  6         | `and` <br> `nand` | Logical AND <br> Logical NOT-AND | |
 * |  5         | `xor` <br> `xnor` | Logical XOR <br> Logical NOT-XOR | |
 * |  4         | `or` <br> `nor` | Logical OR <br> Logical NOT-OR | |
 * |  3         | `? ... :` | Ternary condition | Right-to-Left |
 * |  2         | `=` <br> `+= -=` <br> `*= /= %=` <br> `<<= >>=` <br> `&=` \|`=` `^=` | Assignment <br> Sum, Difference Assignments <br> Product, Quotient, Modulus Assignments <br> Bitshift Assignments <br> Bitwise AND, OR, XOR Assignments | | 
 * |  1         | `,` | Comma | Left-to-Right | 
 */

#ifndef PARSE_H
#define PARSE_H

#include "tree.h"
#include "types/number.h"

/**
 * Operator precedence values. Precedence ranges from 0-15, 15 being the first to be computed
 */
static int operatorPrecedence[] = {
    [T_PLUS] = 12,  [T_MINUS] = 12, [T_STAR] = 13, [T_SLASH] = 13, [T_EXPONENT] = 15,

    [T_EQ] = 9,     [T_NEQ] = 9,

    [T_LT] = 10,    [T_GT] = 10,    [T_LE] = 10,   [T_GE] = 10,

    [T_AND] = 6,    [T_OR] = 4,     [T_XOR] = 5,   [T_NAND] = 6,   [T_NOR] = 4,       [T_XNOR] = 5,

    [T_ASSIGN] = 2,
};

ASTNode* parse_binary_expression(void);
void match_token(TokenType type);
int match_type(Number* out);
TokenType check_for_type(void);
void variable_declaration(void);
ASTNode* function_declaration(void);
ASTNode* function_call_expression(void);
ASTNode* parse_statements(void);

#endif /* PARSE_H */
