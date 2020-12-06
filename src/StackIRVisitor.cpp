#include "StackIRVisitor.h"

#include <iostream>
#include <cstring>
#include <string>

// TODO add StackIRVisitor::
// TODO C++中规定只要返回值不为void的函数没有return，不管返回值有没有被使用，都是未定义行为
// TODO 修复Bug 添加别名之后，就没有原来产生式左边的AST了，也就无法visit了，我按错误的理解写完了实验

// TODO 5.4. 执行完 main 函数但没有通过 return 结束时，返回值默认为 0。
// 看一下函数实现部分有没有什么相关的返回值的要求

// step1, step10
antlrcpp::Any visitProgram(MiniDecafParser::ProgramContext *ctx) {
    local_index = 1;
    for (auto i: ctx->declaration()) {
        if (symbol_table.back()[i->Identifier()->getText()])
            ERROR("Duplicated declaration");
        Var variable;
        variable.index = local_index++;
        variable.init = 0;
        if (i->Equal())
        {
            if (i->expression()) 
                ERROR("must assign const to global variable")
            // 需要检查是否越界
             std::string integer{i->Integer()->getText()};
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
            // global variable是通过额外的汇编在.tss和.data段初始化的
            variable.init = stoi(i->Integer()->getText()); 
        } 
        symbol_table.back()[i->Identifier()->getText()] = variable;
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
    code.push_back("push " + ctx->Integer()->getText());
    return NodeInfo{type:0,// hard coded
                    is_lvalue: false};
}

// step2, step11
antlrcpp::Any visitUnaryOp(MiniDecafParser::UnaryOpContext *ctx) {
    NodeInfo unaryinfo = visit(ctx->unary());
    if (ctx->Minus()) {
        if (unaryinfo.type != 0)
            ERROR("unary -!~ cannot operate on pointer");
        code.push_back("neg");
        return NodeInfo{type: unaryinfo.type,
                        is_lvalue: false};
    }
    else if (ctx->Exclamation()) {
        if (unaryinfo.type != 0)
            ERROR("unary -!~ cannot operate on pointer");
        code.push_back("not");
        return NodeInfo{type: unaryinfo.type,
                        is_lvalue: false};
    }
    else if (ctx->Tilde()) {
        if (unaryinfo.type != 0)
            ERROR("unary -!~ cannot operate on pointer");
        code.push_back("lnot");
        return NodeInfo{type: unaryinfo.type,
                        is_lvalue: false};
    }
    else if (ctx->Ampersand()) {
        if (!unaryinfo.is_lvalue)
            ERROR("& must operate on a left value");
        // 对源操作数类型没有要求，结果类型是 PointerType(源操作数类型)
        return NodeInfo{type: unaryinfo.type + 1,
                        is_lvalue: false};
    } else if (ctx->Asterisk()) {
        if (unaryinfo.type == 0)
            ERROR("* must operate on a pointer");
        code.push_back("load");
        // 操作数类型 e 是指针类型 T*，结果类型是 T
        return NodeInfo{type: unaryinfo.type-1,
                        is_lvalue: true};
    }
}

// step3
 antlrcpp::Any visitAddSub(MiniDecafParser::AddSubContext *ctx) {
    NodeInfo ropearand = visit(ctx->multiplicative());
    NodeInfo lopearand = visit(ctx->additive());
    if (roperand.type != loperand.type)
        ERROR("add sub implicit cast");
    if (ctx->Plus()) 
        code.push_back("add");
    else if (ctx->Minus())
        code.push_back("sub");
    return NodeInfo{type: roperand.type,
                    is_lvalue: false};
  }

  antlrcpp::Any visitMulDiv(MiniDecafParser::MulDivContext *ctx) {
// TODO 修复Bug 添加别名之后，就没有原来产生式左边的AST了，也就无法visit了
    NodeInfo rvalue = visit(ctx->unary());
    NodeInfo lvalue = visit(ctx->multiplicative());
    if (lvalue.type != 0 || rvalue.type != 0)
        ERROR("*/% cannot operate on pointer");
    if (lvalue.type != rvalue.type)
        ERROR("left and right value type mismatch");

    if (ctx->Asterisk()) {
        code.push_back("mul");
        return NodeInfo{type: lvalue.type,
                        is_lvalue: false};
    } 
    else if (ctx->Slash()) {
// TODO 修复Bug 添加别名之后，就没有原来产生式左边的AST了，也就无法visit了
        if (ctx->unary()->postfix()->primary()->Integer())
            if (ctx->unary()->postfix()->primary()->Integer()->getText() = "0")
                ERROR("div 0 error");
        code.push_back("div");
        return NodeInfo{type: lvalue.type,
                        is_lvalue: false};
    }
    else if (ctx->Percent()) {
// TODO 修复Bug 添加别名之后，就没有原来产生式左边的AST了，也就无法visit了
        if (ctx->unary()->postfix()->primary()->Integer())
            if (ctx->unary()->postfix()->primary()->Integer()->getText() = "0")
                ERROR("div 0 error");
        code.push_back("rem");
        return NodeInfo{type: lvalue.type,
                        is_lvalue: false};
    }
  }

// step4
  antlrcpp::Any visitLogicOr(MiniDecafParser::LogicOrContext *ctx) {
    NodeInfo roperand = visit(ctx->logical_and());
    NodeInfo loperand = visit(ctx->logical_or());
    if (roperand.type != loperand.type)
        ERROR("logic or implicit cast");
    if (roperand.type != 0)
        ERROR("logic or cannot operate on pointer");
    code.push_back("lor");
    return NodeInfo{type: 0,is_lvalue: false};
  }

  antlrcpp::Any visitLogicAnd(MiniDecafParser::LogicAndContext *ctx) {
    NodeInfo roperand = visit(ctx->equality());
    NodeInfo loperand = visit(ctx->logical_and());
    if (roperand.type != loperand.type)
        ERROR("logic and implicit cast");
    if (roperand.type != 0)
        ERROR("logic and cannot operate on pointer");
    code.push_back("land");
    return NodeInfo{type: 0,is_lvalue: false};
  }

  antlrcpp::Any visitEqual(MiniDecafParser::EqualContext *ctx) {
    NodeInfo roperand = visit(ctx->relational());
    NodeInfo loperand = visit(ctx->equality());
    if (roperand.type != loperand.type)
        ERROR("== != implicit cast");
    if (ctx->Double_eq()) {
        code.push_back("eq");
        return NodeInfo{type: 0,is_lvalue: false};
    }
    else if (ctx->Exclam_eq()) {
        code.push_back("ne");
        return NodeInfo{type: 0,is_lvalue: false};
    }
  }

  antlrcpp::Any visitLessGreat(MiniDecafParser::LessGreatContext *ctx) {
    NodeInfo roperand = visit(ctx->additive());
    NodeInfo loperand = visit(ctx->relational());
    if (roperand.type != loperand.type)
        ERROR("< > <= >= implicit cast");
    if (roperand.type != 0)
        ERROR("< > <= >= cannot operate on pointer");
    if (ctx->Langle())
    {
        code.push_back("lt");
        return NodeInfo{type: 0,is_lvalue: false};
    }
    else if (ctx->Rangle())
    {
        code.push_back("gt");
        return NodeInfo{type: 0,is_lvalue: false};
    }
    else if (ctx->Langle_eq())
    {
        code.push_back("le");
        return NodeInfo{type: 0,is_lvalue: false};
    }
    else if (ctx->Rangle_eq())
    {
        code.push_back("ge");
        return NodeInfo{type: 0,is_lvalue: false};
    }
  }

// step5, step7, step9, step10, step11
// step5 实验指导 遇到表达式语句时，生成完表达式的 IR 以后记得再生成一个 pop
// 我想的是不需要表达式的返回值的地方都要pop，有expression, declaration两处
// 因为expression是递归的，只能在它的上一步pop，而可以直接在declaration处pop
antlrcpp::Any visitDeclaration(MiniDecafParser::DeclarationContext *ctx)
{
    // TODO 这一步的declaration不包括数组
    if (symbol_table.back()[ctx->Identifier()->getText()])
        ERROR("Duplicated declaration");
    Var variable;
    // 没有复用不冲突的变量的空间，而是全部都在栈上占有位置
    variable.index = local_index++;paranum++;
    variable.type = visit(ctx->type());
    // push variable before assign to support "a=a+1"
    symbol_table.back()[ctx->Identifier()->getText()] = variable;
    if (ctx->Equal())
    {
        visit(ctx->expression);
        code.push_back("load");
        code.push_back("frameaddr " + std::to_string(symbol_table.top().size() - 1));
        code.push_back("store");
// step5 实验指导 遇到表达式语句时，生成完表达式的 IR 以后记得再生成一个 pop
        code.push_back("pop");
    }
    return NodeInfo{type:variable.type,
                    is_lvalue:true};
}

// 这里默认生成的是地址，如果需要右值，要自行添加load指令
antlrcpp::Any visitIdentifier(MiniDecafParser::IdentifierContext *ctx) {
    int find = find_symbol(ctx->Identifier()->getText());
    if (find == -1)
        ERROR("Variable not declared");
    int index = symbol_table_size() - find -1;
    // global varible
    if (index <= symbol_table.front().size()) {
        code.push_back("globaladdr "+ctx->Identifier()->getText())
        //code.push_back("load");
    } else {
        code.push_back("frameaddr " + std::to_string(index));
        //code.push_back("load");
    }
    return NodeInfo{type:find_symbol_type(ctx->Identifier()->getText()),
                    is_lvalue:true};
}

antlrcpp::Any visitAssign(MiniDecafParser::AssignContext *ctx) override {
    NodeInfo rvalue = visit(ctx->expression());
    NodeInfo lvalue = visit(ctx->unary());
    if (!lvalue.is_lvalue)
        ERROR("the left of = must be a left value")
    if (lvalue.type != rvalue.type)
        ERROR("left and right value do not mate at =")
    code.push_back("frameaddr " + std::to_string(index));
    code.push_back("store");
    return NodeInfo{type:lvalue.type,
                    is_lvalue: true};
}

antlrcpp::Any visitSingleExpr(MiniDecafParser::SingleExprContext *ctx) {
    for (auto i: ctx->expression()) {
        visit(i);
// step5 实验指导 遇到表达式语句时，生成完表达式的 IR 以后记得再生成一个 pop
        code.push_back("pop");
    }
}


// step6
  antlrcpp::Any visitIfStmt(MiniDecafParser::IfStmtContext *ctx) {
    std::string end_label = "IF_LABEL_"  + std::to_string(if_order++);
    NodeInfo ifcond = visit(ctx->expression();
    if (ifcond.type != 0)
        ERROR("type of if condition expression must be int");
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
      NodeInfo condcond = visit(ctx->logical_or());
    if (condcond.type != 0)
        ERROR("type of cond condition expression must be int");
      code.push_back("beqz " + end_label);
      NodeInfo loperand = visit(ctx->expression());
      code.push_back("br " + end_label);
      NodeInfo roperand = visit(ctx->conditional());
      code.push_back("label " + end_label);
      if (loperand.type != roperand.type)
        ERROR("the types of expression on both side of : must be the same");
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
// step5 实验指导 遇到表达式语句时，生成完表达式的 IR 以后记得再生成一个 pop
    code.push_back("pop");
    code.push_back("label "+ beginloop_label);
    // TODO expression2 cond 怎么确定有几个变量
    NodeInfo loopcond = visit(ctx->expression());
    if (loopcond.type != 0)
        ERROR("type of loop condition expression must be int");
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
// step5 实验指导 遇到表达式语句时，生成完表达式的 IR 以后记得再生成一个 pop
    code.push_back("pop");
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
// step5 实验指导 遇到表达式语句时，生成完表达式的 IR 以后记得再生成一个 pop
    code.push_back("pop");
    code.push_back("label "+ beginloop_label);
    // TODO expression2 cond 怎么确定有几个变量
    NodeInfo loopcond = visit(ctx->expression());
    if (loopcond.type != 0)
        ERROR("type of loop condition expression must be int");
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
// step5 实验指导 遇到表达式语句时，生成完表达式的 IR 以后记得再生成一个 pop
    code.push_back("pop");
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
    NodeInfo loopcond = visit(ctx->expression());
    if (loopcond.type != 0)
        ERROR("type of loop condition expression must be int");
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
    NodeInfo loopcond = visit(ctx->expression());
    if (loopcond.type != 0)
        ERROR("type of loop condition expression must be int");
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
        f.type = visit(ctx->type());
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
      
    // TODO 将参数入栈，在IR中又需要显示pop吗？（lab9/calling.md）
    for (auto i=ctx->expression_list()->expression().crbegin();
          i!=ctx->expression_list()->expression().crend();--i) {
        visit(i);
    }
    code.push_back("call " + "FUNC_" + ctx->Identifier()->getText());
    return NodeInfo{type=func_table[ctx->Identifier()->getText()].type,
                  is_lvalue = false};
}

// step11
antlrcpp::Any visitCast(MiniDecafParser::CastContext *ctx) {
    // TODO step11
    NodeInfo unaryinfo = visit(ctx->unary());
    return NodeInfo{type=visit(ctx->type()),
                  is_lvalue = unaryinfo.is_lvalue};
}

antlrcpp::Any visitPoinerType(MiniDecafParser::PoinerTypeContext *ctx) {
    // （目前）只有int***这种情况
    int type = 0;
    if (ctx->type()){
        return type += visit(ctx->type());
    } else {
        return  1;
    }
}

antlrcpp::Any visitIntType(MiniDecafParser::IntTypeContext *ctx) {
    return 0;
}

// if e is left value, then (e) is left value
antlrcpp::Any visitAtomParen(MiniDecafParser::AtomParenContext *ctx) {
    return visit(ctx->expression());
}
// step12
// TODO array返回nodeinfo