# pragma once

#include "MiniDecafBaseVisitor.h"
#include <string>

class Visitor: public MiniDecafBaseVisitor {
public:
  antlrcpp::Any visitProgram(MiniDecafParser::ProgramContext *ctx) override;

  //antlrcpp::Any visitFunction(MiniDecafParser::ProgramContext *ctx) override;

  antlrcpp::Any visitStatement(MiniDecafParser::StatementContext *ctx) override;

  antlrcpp::Any visitExpression(MiniDecafParser::ExpressionContext *ctx) override;
private:
    std::string push{"\taddi sp, sp, -4\n"
                    "\tsw a0, (sp)\n"};
};
