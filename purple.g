grammar purple;

tokens {
    T_INTEGER_LITERAL,
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

binaryExpression: additiveExpression
                ;

// This isn't very good
booleanExpression: T_TRUE
                 | T_FALSE
                 ;

expression: binaryExpression
          ;

printStatement: 'print' expression 
              ;

declareStatement: dataType T_IDENTIFIER
                ;

assignStatement: T_IDENTIFIER '=' expression
               ;

ifClause: 'if' '(' booleanExpression ')' statements
        ;

ifStatement : ifClause
            | ifClause 'else' statements
            ;

statementType: printStatement
             | assignStatement
             | declareStatement
             | ifStatement
             ;

statement: statementType ';'
         ;

statements: '{' '}'
          | '{' statement '}'
          | '{' statement statements '}'
          ;