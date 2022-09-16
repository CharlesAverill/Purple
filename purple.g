grammar purple;

tokens {
    T_INTEGER_LITERAL,
    T_IDENTIFIER
}

number: T_INTEGER_LITERAL
        ;

dataType: 'int'
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

expression: binaryExpression
          ;

printStatement: 'print' expression 
              ;

declareStatement: dataType T_IDENTIFIER
                ;

assignStatement: T_IDENTIFIER '=' expression
               ;

statementType: printStatement
             | assignStatement
             | declareStatement
             ;

statement: statementType ';'
         ;

statements: statement
          | statement statements
          ;