# pragma once

#include "MiniDecafBaseVisitor.h"

#include <vector>
#include <string>
#include <map>
#include <list>

class StackIRVisitor: public MiniDecafBaseVisitor {
public:
// step1
  antlrcpp::Any visitProgram(MiniDecafParser::ProgramContext *ctx) override;
antlrcpp::Any visitRetStmt(MiniDecafParser::RetStmtContext *ctx) override;
  antlrcpp::Any visitInteger(MiniDecafParser::IntegerContext *ctx) override;
// step2
antlrcpp::Any visitUnaryOp(MiniDecafParser::UnaryOpContext *ctx) override;
// step3
 antlrcpp::Any visitAddSub(MiniDecafParser::AddSubContext *ctx) override;
  antlrcpp::Any visitMuDiv(MiniDecafParser::MulDivContext *ctx) override;
// step4
  antlrcpp::Any visitLogicOr(MiniDecafParser::LogicOrContext *ctx) override;
  antlrcpp::Any visitLogicAnd(MiniDecafParser::LogicAndContext *ctx) override;
  antlrcpp::Any visitEqual(MiniDecafParser::EqualContext *ctx) override;
  antlrcpp::Any visitLessGreat(MiniDecafParser::LessGreatContext *ctx) override;
// step5 TODO
  antlrcpp::Any visitDeclaration(MiniDecafParser::DeclarationContext *ctx) override;
  antlrcpp::Any visitIdentifier(MiniDecafParser::IdentifierContext *ctx) override;
antlrcpp::Any visitAssign(MiniDecafParser::AssignContext *ctx) override;
// step6
  antlrcpp::Any visitIfStmt(MiniDecafParser::IfStmtContext *ctx) override;
  antlrcpp::Any visitCondExpr(MiniDecafParser::CondExprContext *ctx) override;
// step7
  antlrcpp::Any visitCompound_statement(MiniDecafParser::Compound_statementContext *ctx) override;
// step8
antlrcpp::Any visitForLoop(MiniDecafParser::ForLoopContext *ctx) override;
antlrcpp::Any visitForLoopDec(MiniDecafParser::ForLoopDecContext *ctx) override;
antlrcpp::Any visitWhileLoop(MiniDecafParser::WhileLoopContext *ctx) override;
antlrcpp::Any visitDoLoop(MiniDecafParser::DoLoopContext *ctx) override;
antlrcpp::Any visitBreak(MiniDecafParser::BreakContext *ctx) override;
antlrcpp::Any visitContinue(MiniDecafParser::ContinueContext *ctx) override;

private:

    void ERROR(char* str){
        std::cerr << "[ERROR] " << str << std::endl;
        exit(1);
    }

    int find_symbol(std::string name) {
        int index = 0;
        for (auto i = symbolTable.crbegin(); i != symbolTable.crend(); --i)
        {
            if (i[ctx->Identifier()->getText()])
                return index + i.size() - i[ctx->Identifier()->getText()].index;
            index += i.size();
        }
        return -1;
    }

    int symbol_table_size() {
        int size =0;
        for (auto i: symbolTable)
            size += i.size();
        return size;
    }

private:
    std::vector<std::string> code;
    // step5 为了计算 prologue 中分配栈帧的大小，IR 除了一个指令列表，还要包含一个信息：局部变量的个数
    int local_index=1;
    std::stack<int> local_index_stack;
    std::list<std::map<std::string, Var> symbolTable;
    class Var {
        int index; // start with 1
    };
    // step6
    int if_order = 0;
    // step8
    int loop_order = 0;
    std::stack<std::string> continue_stack;
    std::stack<std::string> break_stack;
};
