grammar MiniDecaf;

import CommonLex;


program
    : function
    ;

function
    : type Identifier '(' ')' '{' statement '}'
    ;

type
    : 'int'
    ;

statement
    : 'return' expression ';'
    ;

expression
    : Integer
    ;
