# pragma once

#include "MiniDecafBaseVisitor.h"

#include <vector>
#include <string>

class StackIRVisitor: public MiniDecafBaseVisitor {
public:
  antlrcpp::Any visitProgram(MiniDecafParser::ProgramContext *ctx) override;
antlrcpp::Any visitRetStmt(MiniDecafParser::RetStmtContext *ctx) override;
  antlrcpp::Any visitInteger(MiniDecafParser::IntegerContext *ctx) override;
antlrcpp::Any visitUnaryOp(MiniDecafParser::UnaryOpContext *ctx) override;
 antlrcpp::Any visitAddSub(MiniDecafParser::AddSubContext *ctx) override;
  antlrcpp::Any visitMulDiv(MiniDecafParser::MulDivContext *ctx) override;
  antlrcpp::Any visitLogicOr(MiniDecafParser::LogicOrContext *ctx) override;
  antlrcpp::Any visitLogicAnd(MiniDecafParser::LogicAndContext *ctx) override;
  antlrcpp::Any visitEqual(MiniDecafParser::EqualContext *ctx) override;
  antlrcpp::Any visitLessGreat(MiniDecafParser::LessGreatContext *ctx) override;

private:
    std::vector<std::string> code;
};
