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
    0, // EOF

    12, // PLUS
    12, // MINUS
    13, // STAR
    13, // SLASH
    15, // EXPONENT

    9, // EQUALS
    9, // NOT EQUALS

    10, // LESS
    10, // GREATER
    10, // LESS EQUAL
    10, // GREATER EQUAL

    6, // AND
    4, // OR
    5, // XOR
    6, // NAND
    4, // NOR
    5, // XNOR

    0, // Literals
    0,  0, 0, 0,

    0, // Types (maybe use these for casting? idk yet)
    0,  0, 0,

    2, // Assignment

    0, // Keywords
    0,  0, 0, 0,

    0, // Miscellaneous
    0,  0, 0, 0, 0, 0, 0,
};

ASTNode* parse_binary_expression(int previous_token_precedence);
void match_token(TokenType type);
NumberType match_type(void);
void variable_declaration(void);
ASTNode* parse_statements(void);

#endif /* PARSE_H */
