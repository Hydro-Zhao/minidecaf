#pragma once

#ifdef LLVM_IR

#include "MiniDecafBaseVisitor.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "util.h"
#include <iostream>
#include <map>
#include <memory>

using namespace llvm;

class IRVisitor : public MiniDecafBaseVisitor {
public:
  IRVisitor()
      : TheContext(TheContext), Builder(Builder), TheModule(TheModule),
        NamedValues(NamedValues) {}

  antlrcpp::Any visitInteger(MiniDecafParser::IntegerContext *ctx) override;
  antlrcpp::Any visitMulDiv(MiniDecafParser::MulDivContext *ctx) override;
  antlrcpp::Any visitAddSub(MiniDecafParser::AddSubContext *ctx) override;
  antlrcpp::Any visitLogicOr(MiniDecafParser::LogicOrContext *ctx) override;
  antlrcpp::Any visitLogicAnd(MiniDecafParser::LogicAndContext *ctx) override;
  antlrcpp::Any visitEqual(MiniDecafParser::EqualContext *ctx) override;
  antlrcpp::Any visitLessGreat(MiniDecafParser::LessGreatContext *ctx) override;
  antlrcpp::Any visitFuncCall(MiniDecafParser::FuncCallContext *ctx) override;
  antlrcpp::Any visitFunction(MiniDecafParser::FunctionContext *ctx) override;

private:
  LLVMContext &TheContext;
  IRBuilder<> &Builder(TheContext);
  std::unique_ptr<Module> &TheModule;
  std::map<std::string, Value *> &NamedValues;
};

#endif