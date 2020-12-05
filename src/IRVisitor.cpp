
#ifdef LLVM_IR
#endif

#include "IRVisitor.h"
#include <cstring>

// For example, by browsing the LLVM language reference you’ll find several 
// other interesting instructions that are really easy to plug into our 
// basic framework.

// ================expression code generation==================

// ------------------numeric literals------------------
antlrcpp::Any IRVisitor::visitInteger(MiniDecafParser::IntegerContext *ctx) {
    return ConstantInt::get(TheContext, APInt(std::stoi(ctx->Integer()->getText())));
}

// ------------------variable------------------
// TODO add loop induction variables in the symbol table and for local variables
// improve the symbol table(also in main.cpp)
// at now, the only values that can be in the NamedValues map are function arguments
Value *VariableExprAST::codegen() {
  // Look this variable up in the function.
  Value *V = NamedValues[Name];
  if (!V)
    LogErrorV("Unknown variable name");
  return V;
}

// ------------------binary expression------------------
// LLVM instructions are constrained by strict rules: 
// for example, the Left and Right operators of an add instruction must have the same type, 
// and the result type of the add must match the operand types.
antlrcpp::Any IRVisitor::visitMulDiv(MiniDecafParser::MulDivContext *ctx) {
    Value *L = visit(ctx->additive());
    Value *R = visit(ctx->multiplicative());
    if (!L || !R)
        Error("invalid TODO");

    if (ctx->Asterisk())
        // TODO Float to Int
        return Builder.CreateFMul(L, R, "multmp");
    else if (ctx->Slash())
        // TODO
        ;
    else if (ctx->Percent)
        // TODO
        ;
    else
        Error("invalid binary operator");
}

antlrcpp::Any IRVisitor::visitAddSub(MiniDecafParser::AddSubContext *ctx)
{
    Value *L = visit(ctx->multiplicative());
    Value *R = visit(ctx->unary());
    if (!L || !R)
        Error("invalid TODO");

    if (ctx->Plus())
        // TODO Float to Int
        return Builder.CreateFAdd(L, R, "addtmp");
    else if (ctx->Minus())
        // TODO Float to Int
        return Builder.CreateFSub(L, R, "subtmp");
    else
        Error("invalid binary operator");
}

antlrcpp::Any IRVisitor::visitLogicOr(MiniDecafParser::LogicOrContext *ctx) override
{
    Value *L = visit(ctx->logical_or());
    Value *R = visit(ctx->logical_and());
    if (!L || !R)
        Error("invalid TODO");

    if (ctx->Double_bar())
        // TODO
        ;
    else
        Error("invalid binary operator");
}

antlrcpp::Any IRVisitor::visitLogicAnd(MiniDecafParser::LogicAndContext *ctx)
{
    if (!L || !R)
        Error("invalid TODO");

    if (ctx->Double_amp())
        // TODO
        ;
    else
        Error("invalid binary operator");
}

antlrcpp::Any IRVisitor::visitEqual(MiniDecafParser::EqualContext *ctx)
{
    Value *L = visit(ctx->equality());
    Value *R = visit(ctx->relational());
    if (!L || !R)
        Error("invalid TODO");

    if (ctx->Double_eq())
        // TODO
        ;
    else if (ctx->Exclam_eq())
        // TODO
        ;
    else
        Error("invalid binary operator");
}

antlrcpp::Any IRVisitor::visitLessGreat(MiniDecafParser::LessGreatContext *ctx)
{
    Value *L = visit(ctx->relational());
    Value *R = visit(ctx->additive());
    if (!L || !R)
        Error("invalid TODO");

    if (ctx->Langle())
        // TODO Float to Int
        return Builder.CreateFCmpULT(L, R, "cmptmp");
    else if (ctx->Rangle())
        // TODO
        ;
    else if (ctx->Langle_eq())
        // TODO
        ;
    else if (ctx->Rangle_eq())
        // TODO
        ;
    else
        Error("invalid binary operator");
}

// ------------------call expression------------------
antlrcpp::Any IRVisitor::visitFuncCall(MiniDecafParser::FuncCallContext *ctx)
{
    // Look up the name in the global module table.
    Function *CalleeF = TheModule->getFunction(ctx->Identifier()->getText());
    if (!CalleeF)
        Error("Unknown function referenced");

    std::vector<ExpressionContext *> &Args = ctx->expression_list()->expression;

    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size())
        Error("Incorrect # arguments passed");

    std::vector<Value *> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i)
    {
        ArgsV.push_back(visit(Args[i]));
        if (!ArgsV.back())
            return nullptr;
    }

    return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

// ================function code generation==================

antlrcpp::Any IRVisitor::visitFunction(MiniDecafParser::FunctionContext *ctx) {
    // 这里是两个函数拼接的(为了支持extern)，有一些冗余的地方
    // -------------------------------------prototype--------------------------
    // TODO double to Int
    std::vector<ExpressionContext *> &Args = ctx->parameter_list()->Identifier();
    // Make the function type:  double(double,double) etc.
    std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(*TheContext));
    FunctionType *FT =
        FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);
    Function *F =
        Function::Create(FT, Function::ExternalLinkage, ctx->Identifier->getText(), TheModule.get());
    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : F->args())
        Arg.setName(Args[Idx++]->getText());
    //return F;

    // -------------------------------------function--------------------------
    // TODO double to Int
    // First, check for an existing function from a previous 'extern' declaration.
    Function *TheFunction = TheModule->getFunction(ctx->Identifier->getText());

    if (!TheFunction)
        TheFunction = F;

    if (!TheFunction)
        return nullptr;

    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto &Arg : TheFunction->args())
        NamedValues[std::string(Arg.getName())] = &Arg;

    if (Value *RetVal = Body->codegen())
    {
        // Finish off the function.
        Builder->CreateRet(RetVal);

        // Validate the generated code, checking for consistency.
        verifyFunction(*TheFunction);

        return TheFunction;
    }

    // Error reading body, remove function.
    TheFunction->eraseFromParent();
    return nullptr;
}






// ================if/then/else code generation==================
// TODO migrate to Antlr

Value *IfExprAST::codegen() {
  Value *CondV = Cond->codegen();
  if (!CondV)
    return nullptr;

  // Convert condition to a bool by comparing non-equal to 0.0.
  CondV = Builder->CreateFCmpONE(
      CondV, ConstantFP::get(*TheContext, APFloat(0.0)), "ifcond");

  Function *TheFunction = Builder->GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  BasicBlock *ThenBB = BasicBlock::Create(*TheContext, "then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(*TheContext, "else");
  BasicBlock *MergeBB = BasicBlock::Create(*TheContext, "ifcont");

  Builder->CreateCondBr(CondV, ThenBB, ElseBB);

  // Emit then value.
  Builder->SetInsertPoint(ThenBB);

  Value *ThenV = Then->codegen();
  if (!ThenV)
    return nullptr;

  Builder->CreateBr(MergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  ThenBB = Builder->GetInsertBlock();

  // Emit else block.
  TheFunction->getBasicBlockList().push_back(ElseBB);
  Builder->SetInsertPoint(ElseBB);

  Value *ElseV = Else->codegen();
  if (!ElseV)
    return nullptr;

  Builder->CreateBr(MergeBB);
  // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
  ElseBB = Builder->GetInsertBlock();

  // Emit merge block.
  TheFunction->getBasicBlockList().push_back(MergeBB);
  Builder->SetInsertPoint(MergeBB);
  PHINode *PN = Builder->CreatePHI(Type::getDoubleTy(*TheContext), 2, "iftmp");

  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);
  return PN;
}


// ================for loop code generation==================
// TODO migrate to Antlr


Value *ForExprAST::codegen() {
  // Emit the start code first, without 'variable' in scope.
  Value *StartVal = Start->codegen();
  if (!StartVal)
    return nullptr;

  // Make the new basic block for the loop header, inserting after current
  // block.
  Function *TheFunction = Builder->GetInsertBlock()->getParent();
  BasicBlock *PreheaderBB = Builder->GetInsertBlock();
  BasicBlock *LoopBB = BasicBlock::Create(*TheContext, "loop", TheFunction);

  // Insert an explicit fall through from the current block to the LoopBB.
  Builder->CreateBr(LoopBB);

  // Start insertion in LoopBB.
  Builder->SetInsertPoint(LoopBB);

  // Start the PHI node with an entry for Start.
  PHINode *Variable =
      Builder->CreatePHI(Type::getDoubleTy(*TheContext), 2, VarName);
  Variable->addIncoming(StartVal, PreheaderBB);

  // Within the loop, the variable is defined equal to the PHI node.  If it
  // shadows an existing variable, we have to restore it, so save it now.
  Value *OldVal = NamedValues[VarName];
  NamedValues[VarName] = Variable;

  // Emit the body of the loop.  This, like any other expr, can change the
  // current BB.  Note that we ignore the value computed by the body, but don't
  // allow an error.
  if (!Body->codegen())
    return nullptr;

  // Emit the step value.
  Value *StepVal = nullptr;
  if (Step) {
    StepVal = Step->codegen();
    if (!StepVal)
      return nullptr;
  } else {
    // If not specified, use 1.0.
    StepVal = ConstantFP::get(*TheContext, APFloat(1.0));
  }

  Value *NextVar = Builder->CreateFAdd(Variable, StepVal, "nextvar");

  // Compute the end condition.
  Value *EndCond = End->codegen();
  if (!EndCond)
    return nullptr;

  // Convert condition to a bool by comparing non-equal to 0.0.
  EndCond = Builder->CreateFCmpONE(
      EndCond, ConstantFP::get(*TheContext, APFloat(0.0)), "loopcond");

  // Create the "after loop" block and insert it.
  BasicBlock *LoopEndBB = Builder->GetInsertBlock();
  BasicBlock *AfterBB =
      BasicBlock::Create(*TheContext, "afterloop", TheFunction);

  // Insert the conditional branch into the end of LoopEndBB.
  Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

  // Any new code will be inserted in AfterBB.
  Builder->SetInsertPoint(AfterBB);

  // Add a new entry to the PHI node for the backedge.
  Variable->addIncoming(NextVar, LoopEndBB);

  // Restore the unshadowed variable.
  if (OldVal)
    NamedValues[VarName] = OldVal;
  else
    NamedValues.erase(VarName);

  // for expr always returns 0.0.
  return Constant::getNullValue(Type::getDoubleTy(*TheContext));
}