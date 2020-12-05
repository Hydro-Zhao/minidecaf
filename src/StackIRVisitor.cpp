#include "StackIRVisitor.h"
#include <iostream>
#include <cstring>

// TODO add StackIRVisitor::
// TODO C++中规定只要返回值不为void的函数没有return，不管返回值有没有被使用，都是未定义行为

// step1, step10
antlrcpp::Any visitProgram(MiniDecafParser::ProgramContext *ctx) {
    local_index = 1;
    for (auto i: ctx->declaration()) {
        if (symbol_table.back()[i->Identifier()->getText()])
            ERROR("Duplicated declaration");
        Var variable;
        variable.index = local_index++;
        symbol_table.back()[i->Identifier()->getText()] = variable;
        std::string value{"0"};
        if (i->Equal())
        {
            if (i->expression()) 
                ERROR("must assign const to global variable")
            // 需要检查是否越界
            visit( i->Integer());
        } 
        code.push_back("globaladdr "+i->Identifier()->getText())
        code.push_back("store");
    }
    for (auto i: ctx->declaration())
        visit(i->function());
    if (!func_table["main"])
        ERROR("program does not have main function")
    return code;
}

antlrcpp::Any visitRetStmt(MiniDecafParser::RetStmtContext *ctx)  {
    visit(ctx->expression());
    // 汇编中ret会跳转到epilogue 
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
    else if (ctx->Slash()) {
      // TODO  3.1. 除以零、模零都是未定义行为
        code.push_back("div");
    }
    else if (ctx->Percent()) {
      // TODO  3.1. 除以零、模零都是未定义行为
        code.push_back("rem");
    }
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

// step5, step7, step9, step10
// TODO BUG step5 实验指导 遇到表达式语句时，生成完表达式的 IR 以后记得再生成一个 pop，保证栈帧要满足的第1条性质
// TODO 这里应该单指赋值语句，但是怎么加pop呢，如果在store后面就不能支持连续赋值了
// TODO 主要要修改assign和declaration
antlrcpp::Any visitDeclaration(MiniDecafParser::DeclarationContext *ctx)
{
    // TODO 这一步的declaration不包括数组
    // TODO step11中还要检查左值
    if (symbol_table.back()[ctx->Identifier()->getText()])
        ERROR("Duplicated declaration");
    Var variable;
    // 没有复用不冲突的变量的空间，而是全部都在栈上占有位置
    variable.index = local_index++;paranum++;
    // push variable before assign to support "a=a+1"
    symbol_table.back()[ctx->Identifier()->getText()] = variable;
    if (ctx->Equal())
    {
        visit(ctx->expression);
        code.push_back("frameaddr " + std::to_string(symbol_table.top().size() - 1));
        code.push_back("store");
    }
}

// TODO 这个需要分左右值吗？
antlrcpp::Any visitIdentifier(MiniDecafParser::IdentifierContext *ctx) {
    int find = find_symbol(ctx->Identifier()->getText());
    if (index == -1)
        ERROR("Variable not declared");
    int index = symbol_table_size() - find -1;
    // global varible
    if (index <= symbol_table.front().size()) {
        code.push_back("globaladdr "+ctx->Identifier()->getText())
        code.push_back("load");
    } else {
    code.push_back("frameaddr " + std::to_string(index));
    code.push_back("load");
    }
}

antlrcpp::Any visitAssign(MiniDecafParser::AssignContext *ctx) override {
    int find = find_symbol(ctx->Identifier()->getText());
    if (index == -1)
        ERROR("Variable not declared");
    int index = symbol_table_size() - find -1;
    // global varible
    if (index <= symbol_table.front().size()) {
        code.push_back("globaladdr "+ctx->Identifier()->getText())
        code.push_back("store");
    } else {
    // TODO step11 left/right value
    visit(ctx->expression());
    code.push_back("frameaddr " + std::to_string(index));
    code.push_back("store");
    }
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
    symbol_table.push_back(std::map<std::string,Var>{});
    visitChildren(ctx);
    symbol_table.pop_back();
    local_index = local_index_stack.top();
    local_index_stack.pop();
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
    symbol_table.push_back(std::map<std::string,Var>{});

    visit(ctx->statement());
    
    // block2 out
    symbol_table.pop_back();
    local_index = local_index_stack.top();
    local_index_stack.pop();

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
    symbol_table.push_back(std::map<std::string,Var>{});

    visit(ctx->declaration());
    code.push_back("label "+ beginloop_label);
    // TODO expression2 cond 怎么确定有几个变量
    visit(ctx->expression());
    code.push_back("beqz " + break_label);

    // block2 in
    local_index_stack.push(local_index);
    local_index = 1;
    symbol_table.push_back(std::map<std::string,Var>{});

    visit(ctx->statement());
    
    // block2 out
    symbol_table.pop_back();
    local_index = local_index_stack.top();
    local_index_stack.pop();

    code.push_back("label "+ continue_label);
    // TODO expression3 post 怎么确定有几个变量
    visit(ctx->expression());
    code.push_back("br " + beginloop_label);
    code.push_back("label " + break_label);

    // block1
    symbol_table.pop_back();
    local_index = local_index_stack.top();
    local_index_stack.pop();

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


// step9, step10
  antlrcpp::Any visitFunction(MiniDecafParser::FunctionContext *ctx) override {
      // TODO 函数声明和定义的参数个数、同一位置的参数类型、以及返回值类型必须相同
      // 多次声明一个函数、以及定义后再声明，均是未定义行为
      // 函数声明中有重名参数，是未定义行为
      // 对于函数定义，参数声明所在作用域就是函数体的块语句对应作用域
      if (symbol_table.front()[ctx->Identifier()->getText()])
        ERROR("function name violate with a global variable");
      if (!func_table[ctx->Identifier()->getText()]) {
        Func f;
        f.paranum = ctx->parameter_list()->Identifier().size();
        // 用这个来确定是否重复定义
        f.framesize = -1;
        func_table[ctx->Identifier()->getText()] = f;
      }
      // 需要确定framesize
      if (func_table[ctx->Identifier()->getText()].framesize == -1) {
          paranum = 0;

          code.push_back("label "+ "FUNC_" + ctx->Identifier()->getText());
          // 汇编中还要生成prologue，需要复制参数

          local_index_stack.push(local_index);
          local_index = 1;
          symbol_table.push_back(std::map<std::string, Var>{});
          visit(parameter_list);
          for (auto i : ctx->compound_statement->block_item())
              visit(i);
          symbol_table.pop_back();
          local_index = local_index_stack.top();
          local_index_stack.pop();

          code.push_back("ret");
          // 汇编中还要生成epilogue，需要复制参数

          func_table[ctx->Identifier()->getText()].framesize = paranum;
      }
      else 
        ERROR("repeat declaration of function");
  }
  // 采用复制caller的参数到callee的栈上的方式，这样方便使用IR指令frameaddr，
  // 感觉如果参数在caller中，IR表现力不够描述这种情况
  // IR不处理复制参数的过程，放到prologue中*
  antlrcpp::Any visitParameter_list(MiniDecafParser::Parameter_listContext *ctx) {
      for (auto i: ctx->Identifier()) {
          symbol_table.back()[i->getText()].index = local_index++;
          paranum++;
      }
  }

// TODO 函数调用也属于expression，如果只是单纯调用，需要pop栈顶的返回值吗
  antlrcpp::Any visitFuncCall(MiniDecafParser::FuncCallContext *ctx) override {
      // C语言中规定只有使用了未定义的返回值才是未定义行为
      // 而C++中规定只要返回值不为void的函数没有return，不管返回值有没有被使用，都是未定义行为
      if (ctx->expression_list()->expression().size != func_table[ctx->Identifier()->getText()])
          ERROR("parameter number is not correct");
      // TODO 同一位置的参数类型也必须相同
        
      // TODO 将参数入栈，这一步需要在IR中体现出来吗？在IR中又需要显示pop吗？（lab9/calling.md）
      for (auto i=ctx->expression_list()->expression().crbegin();
            i!=ctx->expression_list()->expression().crend();--i) {
          visit(i);
      }
      code.push_back("call " + "FUNC_" + ctx->Identifier()->getText());
  }