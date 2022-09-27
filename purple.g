grammar purple;

tokens {
    T_INTEGER_LITERAL,
    T_CHAR_LITERAL,
    T_TRUE,
    T_FALSE,
    T_IDENTIFIER
}

number: T_INTEGER_LITERAL
        ;

booleanLiteral: T_TRUE
              | T_FALSE
              ;

dataType: 'int'
        | 'bool'
        ;

multiplicativeExpression: number
                        | number '*' multiplicativeExpression
                        | number '/' multiplicativeExpression
                        ;

additiveExpression: multiplicativeExpression
                  | additiveExpression '+' multiplicativeExpression
                  | additiveExpression '-' multiplicativeExpression
                  ;

comparativeExpression: binaryExpression '<' binaryExpression
                     | binaryExpression '>' binaryExpression
                     | binaryExpression '<=' binaryExpression
                     | binaryExpression '>=' binaryExpression
                     | binaryExpression '==' binaryExpression
                     | binaryExpression '!=' binaryExpression
                     | binaryExpression // expands to binaryExpression == T_TRUE
                     ;

binaryExpression: additiveExpression
                ;

booleanExpression: T_TRUE
                 | T_FALSE
                 | comparativeExpression
                 ;

expression: binaryExpression
          ;

printStatement: 'print' expression 
              ;

declareStatement: dataType T_IDENTIFIER
                ;

assignStatement: T_IDENTIFIER '=' expression
               ;

ifClause: 'if' '(' comparativeExpression ')' statements
        ;

whileClause: 'while' '(' comparativeExpression ')' statements
           ;

ifStatement: ifClause
           | ifClause 'else' statements
           ;

whileStatement: whileClause
              | whileClause 'else' statements
              ;

forPreamble: statement;
forPostamble: statement;

forStatement: 'for' '(' forPreamble ';' comparativeExpression ';' forPostamble ')' statements 
            // foreach
            ;

statementType: printStatement
             | assignStatement
             | declareStatement
             | ifStatement
             | whileStatement
             | forStatement
             ;

statement: statementType ';'
         ;

statements: '{' '}'
          | '{' statement '}'
          | '{' statement statements '}'
          ;