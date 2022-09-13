grammar purple;

tokens {
    T_INTEGER_LITERAL
}

number: T_INTEGER_LITERAL
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