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
 * | 15         | `++ --` <br> `()` <br> `[]` <br> `.` <br> `->` <br> <i>`(type){list}`</i> </br> | Postfix increment/decrement <br> Function call <br> Array subscript <br> Structure and Union member access <br> Structure and union pointer member access <br> Compound literal | Left-to-Right |
 * | 14         | `**` <br> `++ --` <br> `-` <br> `! ~` <br> <i>`(type)`</i> <br> `*` <br> `&` <br> `bytesize` | Exponent <br> Prefix increment/decrement <br> Unary negative <br> Logical NOT, bitwise NOT <br> Cast <br> Dereference <br> Address-of <br> Size in bytes | Right-to-Left |
 * | 13         | `* / %`  | Multiplication, division, modulus | Left-to-Right |
 * | 12         | `+ -` | Addition, Subtraction | |
 * | 11         | `<< >>` | Bitwise shifts | |
 * | 10         | `< <=` <br> `> >=` | "Less than", "Less than or equal to" relational operators <br> "Greater than", "Greater than or equal to" relational operators | |
 * |  9         | `== !=` | "Is equal", "Is not equal" relational operators | |
 * |  8         | `&` | Bitwise AND | |
 * |  7         | `^` | Bitwise XOR | |
 * |  6         | \| | Bitwise OR | |
 * |  5         | `&&` | Logical AND | |
 * |  4         | \|\| | Logical OR | |
 * |  3         | `? ... :` | Ternary condition | Right-to-Left |
 * |  2         | `=` <br> `+= -=` <br> `*= /= %=` <br> `<<= >>=` <br> `&=` \|`=` `^=` | Assignment <br> Sum, Difference Assignments <br> Product, Quotient, Modulus Assignments <br> Bitshift Assignments <br> Bitwise AND, OR, XOR Assignments | | 
 * |  1         | `,` | Comma | Left-to-Right | 
 */

#ifndef PARSE_H
#define PARSE_H

#include "data.h"
#include "scan.h"
#include "tree.h"

/**
 * Operator precedence values. Precedence ranges from 0-15, 15 being the first to be computed
 */
static int operatorPrecedence[] = {
    0, // EOF

    11, // PLUS
    11, // MINUS
    12, // STAR
    12, // SLASH
    14, // EXPONENT

    8, // EQUALS
    8, // NOT EQUALS

    9, // LESS
    9, // GREATER
    9, // LESS EQUAL
    9, // GREATER EQUAL

    0, // INTLIT
};

ASTNode* parse_binary_expression(int previous_token_precedence);
void parse_statements(void);

#endif /* PARSE_H */
