#include "Visitor.h"
#include <iostream>
#include <cstring>


antlrcpp::Any Visitor::visitInteger(MiniDecafParser::IntegerContext *ctx) {
    // step1 integer must between 0~INT32_MAX
    std::string integer{ctx->Integer()->getText()};
    std::string int_max{std::to_string(INT32_MAX)};
    if (integer.size() > int_max.size()) {
        ERROR("Integer invalid");
    } else if (integer.size() == int_max.size()) {
        for (int i=0; i<integer.size();++i) {
            if (integer[i] > int_max[i]) {
                ERROR("Integer invalid");
            }
        }
    }

    return visitChildren(ctx);
}

antlrcpp::Any Visitor::visitProgram(MiniDecafParser::ProgramContext *ctx) {
    // TODO step1 program must has main function 需要后续函数表的支持
    visitChildren(ctx);
}
 
antlrcpp::Any visitMulDiv(MiniDecafParser::MulDivContext *ctx) {
  if (ctx->Slash()) {
      code.push_back("div");
      // TODO  3.1. 除以零、模零都是未定义行为
        visit(ctx->unary());
  }
  else if (ctx->Percent()) {
      code.push_back("rem");
      // TODO  3.1. 除以零、模零都是未定义行为
        visit(ctx->unary());
  }
}