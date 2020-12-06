# pragma once

#include "MiniDecafBaseVisitor.h"
#include "util.h"

#include <vector>
#include <string>
#include <map>
#include <list>

class Visitor: public MiniDecafBaseVisitor {
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
// step5
  antlrcpp::Any visitDeclaration(MiniDecafParser::DeclarationContext *ctx) override;
  antlrcpp::Any visitIdentifier(MiniDecafParser::IdentifierContext *ctx) override;
antlrcpp::Any visitAssign(MiniDecafParser::AssignContext *ctx) override;
antlrcpp::Any visitSingleExpr(MiniDecafParser::SingleExprContext *ctx) override;
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
// step9
  antlrcpp::Any visitFunction(MiniDecafParser::FunctionContext *ctx) override;
  antlrcpp::Any visitFuncCall(MiniDecafParser::FuncCallContext *ctx) override;
// step11
  antlrcpp::Any visitCast(MiniDecafParser::CastContext *ctx) override;
  antlrcpp::Any visitType(MiniDecafParser::TypeContext *ctx) override;
antlrcpp::Any visitAtomParen(MiniDecafParser::AtomParenContext *ctx) override;
// step12
  antlrcpp::Any visitArry(MiniDecafParser::ArryContext *ctx) override;


private:
    int find_symbol(std::string name) {
        int index = 0;
        for (auto i = symbolTable.crbegin(); i != symbolTable.crend(); --i)
        {
            for (auto j: i) 
                index += j.size();
            if (i[ctx->Identifier()->getText()])
                return index - i[ctx->Identifier()->getText()].index;
        }
        return -1;
    }

    int find_symbol_type(std::string name) {
        for (auto i = symbolTable.crbegin(); i != symbolTable.crend(); --i)
        {
            if (i[ctx->Identifier()->getText()])
                return i[ctx->Identifier()->getText()].type;
        }
        // never, because this function is called after find_symbol
        return -1;
    }

    int symbol_table_size() {
        int size =0;
        for (auto i: symbolTable)
            size += i.size();
        return size;
    }

private:
    std::vector<std::string> code; // 需要导出，可以改成动态内存分配
    // step5, step7
    int local_index=1;
    std::stack<int> local_index_stack;
    std::list<std::map<std::string, Var>> symbol_table; //需要导出
    // step6
    int if_order = 0;
    // step8
    int loop_order = 0;
    std::stack<std::string> continue_stack;
    std::stack<std::string> break_stack;
    // step9
    std::map<std::string, Func> func_table; //需要导出
    int paranum; // 在prameter_list和declaration中会++，但是只是用于对函数的framesize计数
public:
    std::vector<std::string> get_code() {
        return code; // 或许可以改成引用
    }
    std::map<std::string, Var> get_global_symbol_table() {
        return symbol_table.front();
    }
    std::map<std::string, Func> get_func_table() {
        return func_table;
    }
};
