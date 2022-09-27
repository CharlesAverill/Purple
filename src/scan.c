/**
 * @file scan.c
 * @author Charles Averill
 * @brief Functions for lexical scanning from input source files
 * @date 08-Sep-2022
 */

#include <string.h>

#include "data.h"
#include "scan.h"
#include "utils/logging.h"

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
static char skip_whitespace(void)
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
 * @brief Convert a character into its integer form
 * 
 * @param c Character to convert
 * @param base Base to convert to, or -1 if no limit
 * @return int Value of character
 */
static int char_to_int(char c, int base)
{
    int index = index_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", c);
    if (base != -1 && index >= base) {
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Literal of base %d cannot contain character '%c'",
                     base, c);
    }

    if (index == -1) {
        index = index_of("0123456789abcdefghijklmnopqrstuvwxyz", c);
        if (base != -1 && index >= base) {
            syntax_error(D_INPUT_FN, D_LINE_NUMBER,
                         "Literal of base %d cannot contain character '%c'", base, c);
        }
    }

    return index;
}

/**
 * @brief Scan and return an integer literal from the input stream
 * 
 * @param c Current character
 * @return Number Scanned number literal
 */
static Number scan_number_literal(char c)
{
    char number_buffer[MAX_NUMBER_LITERAL_DIGITS + 1];
    int buffer_index = 0;
    Number out = NUMBER_INT(0);
    int current_digit;
    int base = 10;

    // Scan the number into a string
    while (buffer_index < MAX_NUMBER_LITERAL_DIGITS) {
        if (!isdigit(c) && !isalpha(c) && c != NUMBER_LITERAL_BASE_SEPARATOR &&
            c != NUMBER_LITERAL_SPACING_SEPARATOR) {
            break;
        }

        if (c == NUMBER_LITERAL_BASE_SEPARATOR) {
            c = next();
            if (isalpha(c) && !isupper(c)) {
                syntax_error(D_INPUT_FN, D_LINE_NUMBER,
                             "Number literal bases must be of form [1-9|A-Z]");
            }
            base = char_to_int(c, -1);
            if (base == 0) {
                syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Number literals cannot be base 0");
            }
            break;
        }

        if (c == NUMBER_LITERAL_SPACING_SEPARATOR) {
            c = next();
            continue;
        }

        if (c == NUMBER_LITERAL_LONG_SUFFIX) {
            break;
        }

        number_buffer[buffer_index++] = c;
        c = next();
    }

    // Parse the string
    for (int i = 0; i < buffer_index; i++) {
        current_digit = char_to_int(number_buffer[i], base);
        // Check for overflow
        if (out.value * 10 + current_digit < out.value) {
            syntax_error(D_LLVM_FN, D_LINE_NUMBER, "Number literal too big");
        }
        out.value = out.value * base + current_digit;
        // Check for long
        if (out.value >= 2147483648 || out.value < -2147483648) {
            out.type = NT_INT64;
        }
    }

    // Check for a long literal
    if (out.type == NT_INT64 && c != NUMBER_LITERAL_LONG_SUFFIX) {
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Long literals must be suffixed with '%c'",
                     NUMBER_LITERAL_LONG_SUFFIX);
    } else if (c == NUMBER_LITERAL_LONG_SUFFIX) {
        out.type = NT_INT64;
    } else {
        // Loop has terminated at a non-integer value, so put it back
        put_back_into_stream(c);
    }

    return out;

    /*
    int k = 0; // Current digit
    Number out;
    out.type = NT_INT32;
    out.value = 0;

    while ((k = index_of("0123456789", c)) >= 0) {
        if (out.value * 10 + k < out.value) {
            syntax_error(D_LLVM_FN, D_LINE_NUMBER, "Number literal too big");
        }
        out.value = out.value * 10 + k;
        c = next();
    }

    // Check for a long literal
    if (!strcmp(&c, NUMBER_LITERAL_LONG_SUFFIX)) {
        purple_log(LOG_DEBUG, "Parsing long literal");
        out.type = NT_INT64;
    } else {
        // Loop has terminated at a non-integer value, so put it back
        put_back_into_stream(c);
    }

    return out;
    */
}

/**
 * @brief Scan and return an integer literal from the input stream
 * 
 * @param c Current character
 * @return int Scanned integer literal
 */
static char scan_char_literal(char c)
{
    // Should I allow multichar literals?
    return next();
}

/**
 * @brief Determine if a character in an identifier is a permitted character
 * 
 * @param c Character to check
 * @param index Index of character in identifier string
 * @return bool True if character is permitted
 */
static bool is_valid_identifier_char(char c, int index)
{
    return (index != 0 && isdigit(c)) || isalpha(c) || c == '_' || c == '$';
}

/**
 * @brief Scan an alphanumeric identifier
 * 
 * @param c First character in identifier
 * @param buf Buffer to read identifier into
 * @param max_len Maximum length of the identifier
 * @return int Length of the identifier
 */
static int scan_identifier(char c, char* buf, int max_len)
{
    int i = 0;

    while (is_valid_identifier_char(c, i)) {
        if (i >= max_len - 1) {
            syntax_error(D_INPUT_FN, D_LINE_NUMBER,
                         "Identifier name has exceeded maximum length of %d", max_len);
        }

        buf[i++] = c;
        c = next();
    }

    // Loop exits on prohibited identifier character, so put it back
    put_back_into_stream(c);
    buf[i] = '\0';

    // TODO : Check buf against keywords

    return i;
}

/**
 * @brief Retrieve the TokenType value corresponding to a keyword string
 * 
 * @param keyword_string String to convert to TokenType
 * @return TokenType TokenType of the keyword, or 0 if the keyword is not recognized
 */
static TokenType parse_keyword(char* keyword_string)
{
    switch (keyword_string[0]) {
    case 'b':
        if (!strcmp(keyword_string, TTS_BOOL)) {
            return T_BOOL;
        }
        break;
    case 'c':
        if (!strcmp(keyword_string, TTS_CHAR)) {
            return T_CHAR;
        }
        break;
    case 'e':
        if (!strcmp(keyword_string, TTS_ELSE)) {
            return T_ELSE;
        }
    case 'f':
        if (!strcmp(keyword_string, TTS_FALSE)) {
            return T_FALSE;
        }
        if (!strcmp(keyword_string, TTS_FOR)) {
            return T_FOR;
        }
    case 'i':
        if (!strcmp(keyword_string, TTS_IF)) {
            return T_IF;
        }
        if (!strcmp(keyword_string, TTS_INT)) {
            return T_INT;
        }
        break;
    case 'l':
        if (!strcmp(keyword_string, TTS_LONG)) {
            return T_LONG;
        }
        break;
    case 'p':
        if (!strcmp(keyword_string, TTS_PRINT)) {
            return T_PRINT;
        }
        break;
    case 't':
        if (!strcmp(keyword_string, TTS_TRUE)) {
            return T_TRUE;
        }
        break;
    case 'w':
        if (!strcmp(keyword_string, TTS_WHILE)) {
            return T_WHILE;
        }
        break;
    }

    return 0;
}

/**
 * @brief Check if the current character is the start of an integer literal
 * 
 * @param c Character to check
 * @return bool True if the current character is the start of an integer literal
 */
static bool scan_check_integer_literal(char c) { return isdigit(c); }

/**
 * @brief Check if the current character is the start of a character literal
 * 
 * @param c 
 * @return bool True if the current character is the start of a character literal
 */
static bool scan_check_char_literal(char c) { return c == '\''; }

/**
 * @brief Check if the current character is the start of a keyword or identifier
 * 
 * @param c Character to check
 * @return bool True if the current character is the start of a keyword or identifier
 */
static bool scan_check_keyword_identifier(char c) { return is_valid_identifier_char(c, 0); }

/**
 * @brief Scan tokens into the Token struct
 * 
 * @param t Token to scan data into
 * @return bool Returns true if a Token was scanned successfully
 */
bool scan(Token* t)
{
    char c;
    TokenType temp_type;

    // Skip whitespace
    c = skip_whitespace();

    bool switch_matched = true;
    switch (c) {
    case EOF:
        t->type = T_EOF;
        return false;
    case '+':
        t->type = T_PLUS;
        break;
    case '-':
        t->type = T_MINUS;
        break;
    case '*':
        if ((c = next()) == '*') {
            t->type = T_EXPONENT;
        } else {
            put_back_into_stream(c);
            t->type = T_STAR;
        }
        break;
    case '/':
        t->type = T_SLASH;
        break;
    case ';':
        t->type = T_SEMICOLON;
        break;
    case '=':
        if ((c = next()) == '=') {
            t->type = T_EQ;
        } else {
            put_back_into_stream(c);
            t->type = T_ASSIGN;
        }
        break;
    case '!':
        if ((c = next()) == '=') {
            t->type = T_NEQ;
        } else {
            switch_matched = false;
        }
        break;
    case '<':
        if ((c = next()) == '=') {
            t->type = T_LE;
        } else {
            put_back_into_stream(c);
            t->type = T_LT;
        }
        break;
    case '>':
        if ((c = next()) == '=') {
            t->type = T_GE;
        } else {
            put_back_into_stream(c);
            t->type = T_GT;
        }
        break;
    case '(':
        t->type = T_LEFT_PAREN;
        break;
    case ')':
        t->type = T_RIGHT_PAREN;
        break;
    case '{':
        t->type = T_LEFT_BRACE;
        break;
    case '}':
        t->type = T_RIGHT_BRACE;
        break;
    default:
        switch_matched = false;
        break;
    }

    if (switch_matched) {
        return true;
    }

    bool no_switch_match_output = true;
    // Check if c is an integer
    if (scan_check_integer_literal(c)) {
        t->value.number_value = scan_number_literal(c);
        t->type = t->value.number_value.type == NT_INT64 ? T_LONG_LITERAL : T_INTEGER_LITERAL;
    } else if (scan_check_char_literal(c)) {
        t->value.number_value = NUMBER_CHAR(scan_char_literal(c));
        t->type = T_CHAR_LITERAL;
        if (!scan_check_char_literal(next())) {
            syntax_error(D_LLVM_FN, D_LINE_NUMBER,
                         "Multichar literals are not permitted, expected \'");
        }
    } else if (scan_check_keyword_identifier(c)) {
        // Scan identifier string into buffer
        scan_identifier(c, D_IDENTIFIER_BUFFER, D_MAX_IDENTIFIER_LENGTH);

        // Check if identifier is a keyword
        if (temp_type = parse_keyword(D_IDENTIFIER_BUFFER)) {
            t->type = temp_type;
        } else {
            // It's an identifier
            t->type = T_IDENTIFIER;
            strcpy(t->value.symbol_name, D_IDENTIFIER_BUFFER);
        }
    } else {
        no_switch_match_output = false;
        syntax_error(D_INPUT_FN, D_LINE_NUMBER, "Unrecognized token \"%c\"", c);
    }

    // Fill boolean literal values
    if (t->type == T_TRUE) {
        t->value.number_value = NUMBER_BOOL(1);
    } else if (t->type == T_FALSE) {
        t->value.number_value = NUMBER_BOOL(0);
    }

    return no_switch_match_output;
}
