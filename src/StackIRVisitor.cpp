#include "StackIRVisitor.h"
#include <iostream>
#include <cstring>

// add StackIRVisitor::

antlrcpp::Any visitProgram(MiniDecafParser::ProgramContext *ctx) {
    visitChildren(ctx);
    return code;
}

antlrcpp::Any visitRetStmt(MiniDecafParser::RetStmtContext *ctx)  {
    visitChildren(ctx);
    code.push_back("ret");
  }
 
antlrcpp::Any Visitor::visitInteger(MiniDecafParser::IntegerContext *ctx) {
    code.push_back("push " + ctx->Integer()->getText());
    return visitChildren(ctx);
}

antlrcpp::Any visitUnaryOp(MiniDecafParser::UnaryOpContext *ctx) {
    visitChildren(ctx);
    if (ctx->Minus()) 
        code.push_back("neg");
    else if (ctx->Exclamation())
        code.push_back("not");
    else if (ctx->Tilde())
        code.push_back("lnot");
}

 antlrcpp::Any visitAddSub(MiniDecafParser::AddSubContext *ctx) {
    visit(ctx->additive());
    visit(ctx->multiplicative());
    if (ctx->Plus()) 
        code.push_back("add");
    else if (ctx->Minus())
        code.push_back("sub");
  }

  antlrcpp::Any visitMulDiv(MiniDecafParser::MulDivContext *ctx) {
    visit(ctx->multiplicative());
    visit(ctx->unary());
    if (ctx->Asterisk()) 
        code.push_back("mul");
    else if (ctx->Slash())
        code.push_back("div");
    else if (ctx->Percent())
        code.push_back("rem");
  }

  antlrcpp::Any visitLogicOr(MiniDecafParser::LogicOrContext *ctx) {
    visit(ctx->logical_or());
    visit(ctx->logical_and());
    code.push_back("lor");
    return visitChildren(ctx);
  }

  antlrcpp::Any visitLogicAnd(MiniDecafParser::LogicAndContext *ctx) {
    visit(ctx->logical_and());
    visit(ctx->equality());
    code.push_back("land");
    return visitChildren(ctx);
  }

  antlrcpp::Any visitEqual(MiniDecafParser::EqualContext *ctx) {
    visit(ctx->equality());
    visit(ctx->relational());
    if (ctx->Double_eq())
    code.push_back("eq");
    else if (ctx->Exclam_eq())
    code.push_back("ne");
    return visitChildren(ctx);
  }

  antlrcpp::Any visitLessGreat(MiniDecafParser::LessGreatContext *ctx) {
    visit(ctx->relational());
    visit(ctx->additive());
    if (ctx->Langle())
    code.push_back("lt");
    else if (ctx->Rangle())
    code.push_back("gt");
    else if (ctx->Langle_eq())
    code.push_back("le");
    else if (ctx->Rangle_eq())
    code.push_back("ge");
    return visitChildren(ctx);
  }