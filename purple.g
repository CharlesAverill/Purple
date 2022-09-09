grammar purple;

tokens {
    T_INTEGER_LITERAL
}

number: T_INTEGER_LITERAL
        ;

binaryExpression: number
                | binaryExpression '+' binaryExpression
                | binaryExpression '-' binaryExpression
                | binaryExpression '*' binaryExpression
                | binaryExpression '/' binaryExpression
                ;

expression: binaryExpression
          ;