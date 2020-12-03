#include "MiniDecafLexer.h"
#include "MiniDecafParser.h"
#include <iostream>

int main() {
    antlr4::ANTLRInputStream input(std::cin);
    MiniDecafLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    MiniDecafParser parser(&tokens);

    auto tree = parser.prog();
    std::cout << tree->toStringTree(&parser) << std::endl;

    return 0;
}