# pragma once

#include "MiniDecafBaseVisitor.h"
#include <string>
#include <iostream>

class Visitor: public MiniDecafBaseVisitor {
public:
  antlrcpp::Any visitInteger(MiniDecafParser::IntegerContext *ctx);
  antlrcpp::Any visitProgram(MiniDecafParser::ProgramContext *ctx) override;
  antlrcpp::Any visitMulDiv(MiniDecafParser::MulDivContext *ctx) override;

private:
    void ERROR(char* str){
        std::cerr << "[ERROR] " << str << std::endl;
        exit(1);
    }

private:

};
