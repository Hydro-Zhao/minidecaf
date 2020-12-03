#include "Visitor.h"
#include <iostream>


antlrcpp::Any Visitor::visitProgram(MiniDecafParser::ProgramContext *ctx) {
    std::cout << ".section .text\n"
            << ".globl main\n"
            << "main:\n";
    return visitChildren(ctx);
}

//antlrcpp::Any Visitor::visitFunction(MiniDecafParser::ProgramContext *ctx) {
//    if (ctx->Identifier()->getText() != "main") {
//        std::cerr << "function must be main" << std::endl;
//    }
//    return visitChildren(ctx);
//}

antlrcpp::Any Visitor::visitStatement(MiniDecafParser::StatementContext *ctx) {
    visitChildren(ctx);
    std::cout << "\tret";
}

antlrcpp::Any Visitor::visitExpression(MiniDecafParser::ExpressionContext *ctx) {
    if (stoi(ctx->Integer()->getText()) > INT32_MAX || stoi(ctx->Integer()->getText()) < 0){
        std::cerr << "Integer not valid" << std::endl;
    }
    std::cout << "\tli a0, " << ctx->Integer()->getText() << "\n" << push;
}