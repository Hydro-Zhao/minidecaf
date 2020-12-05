# pragma once

#include "MiniDecafBaseVisitor.h"

class Visitor: public MiniDecafBaseVisitor {
public:
  antlrcpp::Any visitProgram(MiniDecafParser::ProgramContext *ctx) override;

  antlrcpp::Any visitFunction(MiniDecafParser::FunctionContext *ctx) override;

  antlrcpp::Any visitStatement(MiniDecafParser::StatementContext *ctx) override;

  antlrcpp::Any visitExpression(MiniDecafParser::ExpressionContext *ctx) override;

  antlrcpp::Any visitUnary(MiniDecafParser::UnaryContext *ctx) override;

  antlrcpp::Any visitInteger(MiniDecafParser::IntegerContext *ctx);

private:
    void ERROR(char* str){
        std::cerr << "[ERROR] " << str << std::endl;
        exit(1);
    }

private:

};
