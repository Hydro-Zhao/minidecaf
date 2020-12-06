grammar MiniDecaf;

import CommonLex;

program
    : (function | declaration)*
    ;

function
    : type Identifier '(' parameter_list ')' (compound_statement | ';')
    ;

type
    : 'int'
    | type '*'
    ;

parameter_list
    : (type Identifier (',' type Identifier)*)?
    ;

compound_statement
    : '{' block_item* '}'
    ;

block_item
    : statement
    | declaration
    ;

statement
    : 'return' expression ';'                                               # retStmt
    | expression? ';'                                                       # singleExpr
    | 'if' '(' expression ')' statement ('else' statement)?                 # ifStmt
    | compound_statement                                                    # block
    | 'for' '(' expression? ';' expression? ';' expression? ')' statement   # forLoop
    | 'for' '(' declaration expression? ';' expression? ')' statement       # forLoopDec  
    | 'while' '(' expression ')' statement                                  # whileLoop
    | 'do' statement 'while' '(' expression ')' ';'                         # doLoop
    | 'break' ';'                                                           # break
    | 'continue' ';'                                                        # continue
    ;

declaration
    : type Identifier ('[' Integer ']')* ('=' expression)? ';'
    ;

expression_list
    : (expression (',' expression)*)?
    ;

expression
    : assignment
    ;

assignment
    : conditional                                                           # cond
    | unary '=' expression                                                  # assign
    ;

conditional
    : logical_or                                                            # nopor
    | logical_or '?' expression ':' conditional                             # condExpr
    ;

logical_or
    : logical_and                                                           # nopand
    | logical_or '||' logical_and                                           # logicOr
    ;

logical_and
    : equality                                                              # nopequal
    | logical_and '&&' equality                                             # logicAnd
    ;

equality
    : relational                                                            # noprel
    | equality ('=='|'!=') relational                                       # equal
    ;

relational
    : additive                                                              # nopadd
    | relational ('<'|'>'|'<='|'>=') additive                               # lessGreat
    ;

additive
    : multiplicative                                                        # nopmul
    | additive ('+'|'-') multiplicative                                     # addSub
    ;

multiplicative
    : unary                                                                 # nopunary
    | multiplicative ('*'|'/'|'%') unary                                    # mulDiv
    ;

unary
    : postfix                                                               # noppostfix
    | ('-'|'~'|'!'|'&'|'*') unary                                           # unaryOp
    | '(' type ')' unary                                                    # cast
    ;

postfix
    : primary                                                               # nopprimary
    | Identifier '(' expression_list ')'                                    # funcCall
    | postfix '[' expression ']'                                            # arry
    ;

primary
    : Integer                                                               # integer
    | '(' expression ')'                                                    # atomParen
    | Identifier                                                            # identifier
    ;
