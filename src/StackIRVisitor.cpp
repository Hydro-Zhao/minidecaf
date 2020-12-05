#include "StackIRVisitor.h"
#include <iostream>
#include <cstring>

// add StackIRVisitor::

// step1
antlrcpp::Any visitProgram(MiniDecafParser::ProgramContext *ctx) {
    visitChildren(ctx);
    return code;
}

antlrcpp::Any visitRetStmt(MiniDecafParser::RetStmtContext *ctx)  {
    visit(ctx->expression());
    // TODO step5 需要注意的是，IR 的 ret 指令不能直接 jr ra 了，而是要跳转执行 epilogue，即 j FUNCNAME_epilogue。
    code.push_back("ret");
  }
 
antlrcpp::Any Visitor::visitInteger(MiniDecafParser::IntegerContext *ctx) {
    code.push_back("push " + ctx->Integer()->getText());
}

// step2
antlrcpp::Any visitUnaryOp(MiniDecafParser::UnaryOpContext *ctx) {
    visit(ctx->unary());
    if (ctx->Minus()) 
        code.push_back("neg");
    else if (ctx->Exclamation())
        code.push_back("not");
    else if (ctx->Tilde())
        code.push_back("lnot");
}

// step3
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

// step4
  antlrcpp::Any visitLogicOr(MiniDecafParser::LogicOrContext *ctx) {
    visit(ctx->logical_or());
    visit(ctx->logical_and());
    code.push_back("lor");
  }

  antlrcpp::Any visitLogicAnd(MiniDecafParser::LogicAndContext *ctx) {
    visit(ctx->logical_and());
    visit(ctx->equality());
    code.push_back("land");
  }

  antlrcpp::Any visitEqual(MiniDecafParser::EqualContext *ctx) {
    visit(ctx->equality());
    visit(ctx->relational());
    if (ctx->Double_eq())
        code.push_back("eq");
    else if (ctx->Exclam_eq())
        code.push_back("ne");
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
  }

// step5, step7
// TODO BUG step5 实验指导 遇到表达式语句时，生成完表达式的 IR 以后记得再生成一个 pop，保证栈帧要满足的第1条性质
// 这里应该单指赋值语句，但是怎么加pop呢，如果在store后面就不能支持连续赋值了
// 主要要修改assign和declaration
antlrcpp::Any visitDeclaration(MiniDecafParser::DeclarationContext *ctx)
{
    // TODO 这一步的declaration不包括数组
    // TODO step11中还要检查左值
    if (symbolTable.back()[ctx->Identifier()->getText()])
        ERROR("Duplicated declaration");
    Var variable;
    // 没有使用节省空间的存储方式
    variable.index = local_index++;
    // push variable before assign to support "a=a+1"
    symbolTable.back()[ctx->Identifier()->getText()] = variable;
    if (ctx->Equal())
    {
        visit(ctx->expression);
        code.push_back("frameaddr " + std::to_string(symbolTable.top().size() - 1));
        code.push_back("store");
    }
}

// 这个需要分左右值吗？
antlrcpp::Any visitIdentifier(MiniDecafParser::IdentifierContext *ctx) {
    int find = find_symbol(ctx->Identifier()->getText());
    if (index == -1)
        ERROR("Variable not declared");
    int index = symbol_table_size() - find -1;
    code.push_back("frameaddr " + std::to_string(index));
    code.push_back("load");
}

antlrcpp::Any visitAssign(MiniDecafParser::AssignContext *ctx) override {
    int find = find_symbol(ctx->Identifier()->getText());
    if (index == -1)
        ERROR("Variable not declared");
    int index = symbol_table_size() - find -1;
    // step11 left/right value
    visit(ctx->expression());
    code.push_back("frameaddr " + std::to_string(index));
    code.push_back("store");
}


// step6
  antlrcpp::Any visitIfStmt(MiniDecafParser::IfStmtContext *ctx) {
    std::string end_label = "IF_LABEL_"  + std::to_string(if_order++);
    visit(ctx->expression();
    if (ctx->Else(0)) {
        std::string else_label = "IF_LABEL_"  + std::to_string(if_order++);
        code.push_back("beqz "+ else_labe));
        visit(ctx->statement(0));
        code.push_back("br "+ end_label);
        code.push_back("label " + else_label);
        visit(ctx->statement(1));
    } else {
        code.push_back("beqz "+ end_label);
        visit(ctx->statement(0));
    }
    code.push_back("label " + end_label);

  }
  
  antlrcpp::Any visitCondExpr(MiniDecafParser::CondExprContext *ctx) {
      std::string end_label = "IF_LABEL_"  + std::to_string(if_order++);
      visit(ctx->logical_or());
      code.push_back("beqz " + end_label);
      visit(ctx->expression());
      code.push_back("br " + end_label);
      visit(ctx->conditional());
      code.push_back("label " + end_label);
  }


// step7
antlrcpp::Any visitCompound_statement(MiniDecafParser::Compound_statementContext *ctx) {
    local_index_stack.push(local_index);
    local_index = 1;
    symbolTable.push_back(std::map<std::string,Var>{});
    visitChildren(ctx);
    symbolTable.pop_back();
    local_index = local_index_stack.top();
    local_index_stack(local_index);
}

// step8
antlrcpp::Any visitForLoop(MiniDecafParser::ForLoopContext *ctx) {
    std::string beginloop_label = "BEGINLOOP_LABEL_"  + std::to_string(loop_order++);

    std::string break_label = "BREAK_LABEL_"  + std::to_string(loop_order++);
    break_stack.push(break_label);
    std::string continue_label = "CONTINUE_LABEL_"  + std::to_string(loop_order++);
    continue_stack.push(continue_label);

    // TODO expression1 pre 怎么确定有几个变量
    visit(ctx->expression());
    code.push_back("label "+ beginloop_label);
    // TODO expression2 cond 怎么确定有几个变量
    visit(ctx->expression());
    code.push_back("beqz " + break_label);

    // block2 in
    local_index_stack.push(local_index);
    local_index = 1;
    symbolTable.push_back(std::map<std::string,Var>{});

    visit(ctx->statement());
    
    // block2 out
    symbolTable.pop_back();
    local_index = local_index_stack.top();
    local_index_stack(local_index);

    code.push_back("label "+ continue_label);
    // TODO expression3 post 怎么确定有几个变量
    visit(ctx->expression());
    code.push_back("br " + beginloop_label);
    code.push_back("label " + break_label);

    break_stack.pop();
    continue_stack.pop();
}

antlrcpp::Any visitForLoopDec(MiniDecafParser::ForLoopDecContext *ctx) {
    std::string beginloop_label = "BEGINLOOP_LABEL_"  + std::to_string(loop_order++);

    std::string break_label = "BREAK_LABEL_"  + std::to_string(loop_order++);
    break_stack.push(break_label);
    std::string continue_label = "CONTINUE_LABEL_"  + std::to_string(loop_order++);
    continue_stack.push(continue_label);

    // block1
    local_index_stack.push(local_index);
    local_index = 1;
    symbolTable.push_back(std::map<std::string,Var>{});

    visit(ctx->declaration());
    code.push_back("label "+ beginloop_label);
    // TODO expression2 cond 怎么确定有几个变量
    visit(ctx->expression());
    code.push_back("beqz " + break_label);

    // block2 in
    local_index_stack.push(local_index);
    local_index = 1;
    symbolTable.push_back(std::map<std::string,Var>{});

    visit(ctx->statement());
    
    // block2 out
    symbolTable.pop_back();
    local_index = local_index_stack.top();
    local_index_stack(local_index);

    code.push_back("label "+ continue_label);
    // TODO expression3 post 怎么确定有几个变量
    visit(ctx->expression());
    code.push_back("br " + beginloop_label);
    code.push_back("label " + break_label);

    // block1
    symbolTable.pop_back();
    local_index = local_index_stack.top();
    local_index_stack(local_index);

    break_stack.pop();
    continue_stack.pop();
}

antlrcpp::Any visitWhileLoop(MiniDecafParser::WhileLoopContext *ctx) {
    std::string beginloop_label = "BEGINLOOP_LABEL_"  + std::to_string(loop_order++);

    std::string break_label = "BREAK_LABEL_"  + std::to_string(loop_order++);
    break_stack.push(break_label);
    std::string continue_label = "CONTINUE_LABEL_"  + std::to_string(loop_order++);
    continue_stack.push(continue_label);

    code.push_back("label "+ beginloop_label);
    visit(ctx->expression());
    code.push_back("beqz " + break_label);
    visit(ctx->statement());
    code.push_back("label "+ continue_label);
    code.push_back("br " + beginloop_label);
    code.push_back("label " + break_label);

    break_stack.pop();
    continue_stack.pop();
}

antlrcpp::Any visitDoLoop(MiniDecafParser::DoLoopContext *ctx) {
    std::string beginloop_label = "BEGINLOOP_LABEL_"  + std::to_string(loop_order++);

    std::string break_label = "BREAK_LABEL_"  + std::to_string(loop_order++);
    break_stack.push(break_label);
    std::string continue_label = "CONTINUE_LABEL_"  + std::to_string(loop_order++);
    continue_stack.push(continue_label);

    code.push_back("label "+ beginloop_label);
    visit(ctx->statement());
    code.push_back("label "+ continue_label);
    visit(ctx->expression());
    code.push_back("beqz " + break_label);
    code.push_back("br " + beginloop_label);
    code.push_back("label " + break_label);

    break_stack.pop();
    continue_stack.pop();
}


//循环语句的语义和 C 的也相同，并且我们要检查 break/continue 不能出现在循环外。
antlrcpp::Any visitBreak(MiniDecafParser::BreakContext *ctx) {
    if (!break_stack.top())
        ERROR("break out of loop");
    code.push_back("br " + break_stack.top());
}

antlrcpp::Any visitContinue(MiniDecafParser::ContinueContext *ctx) {
    if (!continue_stack.top())
        ERROR("continue out of loop");
    code.push_back("br " + continue_stack.top());
}

