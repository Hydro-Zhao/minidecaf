#include "MiniDecafLexer.h"
#include "MiniDecafParser.h"
#include "IRVisitor.h"
#include "Visitor.h"
#include "util.h"

#include <iostream>
#include <mpa>
#include <vector>

#ifdef LLVM_IR
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
#include "IRVisitor.h"

using namespace llvm;
#endif

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./MiniDecaf test.c" << std::endl;
        return 0;
    }
    std::ifstream stream;
    stream.open(argv[1]);
    antlr4::ANTLRInputStream input(stream);
    MiniDecafLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    MiniDecafParser parser(&tokens);

    auto tree = parser.program();
    //std::cout << tree->toStringTree(&parser) << std::endl;

    Visitor visitor;
    visitor.visitProgram(tree); 
    std::vector<std::string> code = visitor.get_code(); // 用于汇编生成
    std::map<std::string, Var> global_symbol_table = visitor.get_global_symbol_table(); // 用于汇编生成
    std::map<std::string, Func> func_table = visitor.get_func_table();

#ifdef LLVM_IR
    LLVMContext TheContext;
    IRBuilder<> Builder(TheContext);
    std::unique_ptr<Module> TheModule;
    // TODO improve this symbol table
    std::map<std::string, Value *> NamedValues;
    // 这个要针对visitor模式处理一下，
    // 需要在IRVisitor中添加该项或者换一种方式来保存函数信息
    // 目前针对这个的修改只有这一部分，后续要修改IRVisitor中的有关部分，
    // 主要就是确定函数保存结构和getFunction函数
    std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

    InitializeModuleAndPassManager();

    IRVisitor irvisitor{TheContext, Builder, TheModule, NameValues};
    irvisitor.visitProgram(tree);

    // Print out all of the generated code.
    TheModule->print(errs(), nullptr);

    // Initialize the target registry etc.
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    auto TargetTriple = sys::getDefaultTargetTriple();
    TheModule->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target)
    {
        errs() << Error;
        return 1;
    }

    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TheTargetMachine =
        Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    TheModule->setDataLayout(TheTargetMachine->createDataLayout());

    auto Filename = "output.o";
    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

    if (EC)
    {
        errs() << "Could not open file: " << EC.message();
        return 1;
    }

    legacy::PassManager pass;
    auto FileType = CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
    {
        errs() << "TheTargetMachine can't emit a file of this type";
        return 1;
    }

    pass.run(*TheModule);
    dest.flush();

    outs() << "Wrote " << Filename << "\n";
#endif

    return 0;
}