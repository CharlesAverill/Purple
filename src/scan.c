/**
 * @file scan.c
 * @author Charles Averill
 * @brief Functions for lexical scanning from input source files
 * @date 08-Sep-2022
 */

#include "scan.h"

/**
 * @brief Get the next valid character from the current input file
 * 
 * @return char Next valid character from the current input file
 */
static char next(void)
{
    // Get next valid character from file

    char c;

    // If we have to put a character back into the stream
    if (D_PUT_BACK) {
        c = D_PUT_BACK;
        D_PUT_BACK = 0;
        return c;
    }

    // Get next character from file
    c = fgetc(D_INPUT_FILE);

    // Check line increment
    if (c == '\n') {
        D_LINE_NUMBER++;
    }

    return c;
}

/**
 * @brief Put a character back into the input stream
 * 
 * @param c Character to be placed into the input stream
 */
static void put_back_into_stream(char c) { D_PUT_BACK = c; }

/**
 * @brief Skips whitespace tokens
 * 
 * @return char The next non-whitespace Token
 */
static char skip(void)
{
    char c;

    // Get next character until not hit space, tab, newline, carriage return, form feed
    do {
        c = next();
    } while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f');

    return c;
}

/**
 * @brief Return the index of char c in string s
 * 
 * @param s String to search in
 * @param c Character to search for
 * @return int Index of c in s if s contains c, else -1
 */
static int index_of(char* s, int c)
{
    char* p = strchr(s, c);

    if (p) {
        return p - s;
    } else {
        return -1;
    }
}

/**
 * @brief Scan and return an integer literal from the input stream
 * 
 * @param c Current character
 * @return int Scanned integer literal
 */
static int scanint(char c)
{
    int k = 0;   // Current digit
    int val = 0; // Output

    while ((k = index_of("0123456789", c)) >= 0) {
        val = val * 10 + k;
        c = next();
    }

    // Loop has terminated at a non-integer value, so put it back
    put_back_into_stream(c);

    return val;
}

/**
 * @brief Scan tokens into the Token struct
 * 
 * @param t Token to scan data into
 * @return bool Returns true if a Token was scanned successfully
 */
bool scan(Token* t)
{
    char c;

    // Skip whitespace
    c = skip();

    switch (c) {
    case EOF:
        t->type = T_EOF;
        return 0;
    case '+':
        t->type = T_PLUS;
        break;
    case '-':
        t->type = T_MINUS;
        break;
    case '*':
        t->type = T_STAR;
        break;
    case '/':
        t->type = T_SLASH;
        break;
    default:
        // Check if c is an integer
        if (isdigit(c)) {
            t->value = scanint(c);
            t->type = T_INTEGER_LITERAL;
            break;
        } else {
            fatal(1, "Unrecognized character %c on line %d\n", c, D_LINE_NUMBER);
        }
    }

    return 1;
}
