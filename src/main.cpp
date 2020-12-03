#include "MiniDecafLexer.h"
#include "MiniDecafParser.h"
#include "Visitor.h"
#include <iostream>

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

    return 0;
}