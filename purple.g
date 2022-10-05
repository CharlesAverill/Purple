grammar purple;

tokens {
    T_IDENTIFIER,
    T_INTEGER_LITERAL,
    T_CHAR_LITERAL
}

booleanLiteral: 'true'
              | 'false'
              ;

type: 'void'
    | 'bool'
    | 'char'
    | 'int'
    | 'long'
    ;

assignmentOperator: '='
                  ;

literal: T_INTEGER_LITERAL
       | T_CHAR_LITERAL
       | booleanLiteral
       ;

primaryExpression: T_IDENTIFIER
                 | literal
                 ;

postfixExpression: primaryExpression
                 ;

unaryExpression: postfixExpression
               ;

castExpression: unaryExpression
              ;

multiplicativeExpression: castExpression
                        | multiplicativeExpression '*' castExpression
                        | multiplicativeExpression '/' castExpression
                        ;

additiveExpression: multiplicativeExpression
                  | additiveExpression '+' multiplicativeExpression
                  | additiveExpression '-' multiplicativeExpression
                  ;

shiftExpression: additiveExpression
               ;

relationalExpression: shiftExpression
                    | relationalExpression '<' shiftExpression
                    | relationalExpression '>' shiftExpression
                    | relationalExpression '<=' shiftExpression
                    | relationalExpression '>=' shiftExpression
                    ;

equalityExpression: relationalExpression
                  | equalityExpression '==' relationalExpression
                  | equalityExpression '!=' relationalExpression
                  ;

andLogicalExpression: equalityExpression
                    | andLogicalExpression 'and' equalityExpression
                    ;

xorLogicalExpression: andLogicalExpression
                    | xorLogicalExpression 'xor' andLogicalExpression
                    ;

orLogicalExpression: xorLogicalExpression
                   | orLogicalExpression 'or' xorLogicalExpression
                   ;

constantExpression: orLogicalExpression
                  ;

assignmentExpression: orLogicalExpression
                    | unaryExpression assignmentOperator assignmentExpression
                    ;

booleanExpression: booleanLiteral
                 | constantExpression
                 ;

expression: booleanExpression
          ;

printStatement: 'print' expression 
              ;

declareStatement: type T_IDENTIFIER
                ;

ifStatement: 'if' '(' expression ')' statements
           | 'if' '(' expression ')' statements 'else' statements
           ;

whileStatement: 'while' '(' expression ')' statements
              | 'while' '(' expression ')' statements 'else' statements
              ;

forStatement: 'for' '(' expression ';' expression ';' expression ')' statements
            ;

loopStatement: whileStatement
             | forStatement
             ;

functionCallStatement: T_IDENTIFIER '(' expression ')'
                     ;

statementType: printStatement
             | assignmentExpression
             | declareStatement
             | ifStatement
             | loopStatement
             | functionCallStatement
             ;

statement: statementType ';'
         ;

statements: '{' '}'
          | '{' statement '}'
          | '{' statement statements '}'
          ;

functionDeclaration: 'void' T_IDENTIFIER '(' 'void' ')' statements;