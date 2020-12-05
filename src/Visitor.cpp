#include "Visitor.h"
#include <iostream>
#include <cstring>


antlrcpp::Any Visitor::visitProgram(MiniDecafParser::ProgramContext *ctx) {
    // start of program
    // bss, data, text
    visitChildren(ctx);
    // this will return to main
    // TODO
    return 0;
}
 
antlrcpp::Any Visitor::visitFunction(MiniDecafParser::FunctionContext *ctx) {
    // step1 program must has main function
    // TODO how to deal with multi function, 支持函数之后要从函数表中查找 
    if (ctx->Identifier()->getText() != std::string("main")) {
        ERROR("function must be main");
    }
    
    return visitChildren(ctx);
}

antlrcpp::Any Visitor::visitStatement(MiniDecafParser::StatementContext *ctx) {
    return visitChildren(ctx);
}

antlrcpp::Any Visitor::visitExpression(MiniDecafParser::ExpressionContext *ctx) {
    return visitChildren(ctx);
}

antlrcpp::Any Visitor::visitInteger(MiniDecafParser::IntegerContext *ctx) {
    // step1 integer must between 0~INT32_MAX
    std::string integer{ctx->Integer()->getText()};
    std::string int_max{std::to_string(INT32_MAX)};
    if (integer.size() > int_max.size()) {
        ERROR("Integer not valid");
    } else if (integer.size() == int_max.size()) {
        for (int i=0; i<integer.size();++i) {
            if (integer[i] > int_max[i]) {
                ERROR("Integer not valid");
            }
        }
    }

    return visitChildren(ctx);
}

antlrcpp::Any Visitor::visitUnary(MiniDecafParser::UnaryContext *ctx) {
    
    return visitChildren(ctx);
}