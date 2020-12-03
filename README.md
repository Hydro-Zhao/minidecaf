2020秋编译原理实验（与往年的Decaf没有关系）

## next

LLVM tutorial包含前端，后端，汇编器的实现指导

结合LLVM IR，加入JIT等

后端优化

## 运行

按照实验手册安装Antlr，无需置环境变量，但是需要将Antlr包放到/usr/local/lib

按照官方Cpp runtime构建方法编译，并将include和lib复制到/usr/local/include和/usr/local/lib中（不用运行`ldconfig`）

同时提供Makefile和CMakeList.txt。主要是做了这么多PA，还没有自己从头开始写的作业，我也还没正真写过Makefile，就顺便写了。CMake的tutorial看的有点昏，不知道讲什么（我自己没有太多项目经验，需求太少），make的文档也很全，但是我觉得我更需要的是那种Makefile C/Cpp best practice的东西。

[minidecaf相关文档](https://github.com/decaf-lang/minidecaf/tree/dev-specs/doc)，包含minidecaf实验演变过程，minidecaf语法，riscv指令介绍，riscv汇编介绍。

## 文档摘要及实验记录

lexer

parser

名称解析

类型检查

生成IR

生成汇编

### step0 第一个编译器

#### 基础操作及环境配置

riscv toolchain可以到SiFive上下载

```
riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -O3

qemu-riscv32

cd /usr/local/lib
sudo curl -D https://www.antlr.org/download/antlr-4.8-complete.jar
export CLASSPATH=".:/usr/local/lib/antlr-4.8-complete.jar:$CLASSPATH"
alias antlr4='java -jar /usr/local/lib/antlr-4.8-complete.jar'
alias grun='java org.antlr.v4.gui.TestRig'
```

#### 测试

通过 git clone 把[测例](https://github.com/decaf-lang/minidecaf-tests)和某个参考实现克隆到同一个目录下面

按照参考实现的 README 配置好它的环境

按照测例的 README 运行测试

### step1

#### Vistor模式

https://github.com/decaf-lang/minidecaf-tutorial/blob/master/docs/lab1/visitor.md

#### Antlr

了解Antlr词法和语法分析的书写要求（语法规则，规则命名...）

ANTLR 默认生成 listener，它允许你在遍历 AST 过程进入结点和离开结点的时候运行一些代码，但我们不用 listener，我们使用 visitor。

除 Visitor 之外，你也可以通过产生式动作使用 ANTLR。若要使用产生式动作，你需要在产生式下方添加一对大括号，并在大括号里用你选择的编程语言编写动作(按下述参数告诉 ANTLR 既不用生成 Visitor 也不用生成 Listenerantlr4 Expr.g4 -no-listener -no-visitor)。

[官方安装方法](https://github.com/antlr/antlr4/tree/master/runtime/Cpp)

[InstallAntlr4Cpp](http://www.cs.sjsu.edu/~mak/tutorials/InstallANTLR4Cpp.pdf)多了将库复制到/usr/local/include和/usr/local/lib的步骤，最后还要运行`sudo ldconfig`

```
$ antlr4 Expr.g4 -Dlanguage=Cpp
$ ls ExprParser.cpp ExprLexer.cpp  # 生成了 C++ 的 lexer 和 parser
ExprLexer.cpp  ExprParser.cpp
$ g++ main.cpp ExprLexer.cpp ExprParser.cpp 你的antlr路径/antlr4-cpp-runtime/dist/libantlr4-runtime.a -I 你的antlr路径/antlr4-cpp-runtime/runtime/src -o main
$ ./main <input
(expr (add (add (add (mul (atom ( (expr (add (add (mul (atom 1))) + (mul (atom 3)))) )) (mulOp *) (atom 4))) - (mul (atom 3))) - (mul (atom 3))))


main.cpp

#include "ExprLex.h"
#include "ExprParser.h"
#include <iostream>

int main() {
    antlr4::ANTLRInputStream input(std::cin);
    ExprLex lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    ExprParser parser(&tokens);
    auto tree = parser.expr();
    std::cout << tree->toStringTree(&parser) << std::endl;
    return 0;
}
```

#### 手写lexer和parser

https://github.com/decaf-lang/minidecaf-tutorial/blob/master/docs/lab1/manual-parser.md

可以参考llvm tutorial或其它博客

lexer：返回token list。包含token，tokenkind等结构体。定义了AST node结构体。实现了获得当前token，进入下一个token等接口

parser：解析的过程是对产生式的还原。有最上层解析函数。可分为非终结符解析函数（会消耗多个token，是由其它的非终结符函数、终结符函数和处理AST节点的语句构成（还可同时完成名称解析或类型检查）），终结符解析函数（可以输出错误信息）。

#### IR

IR有很多种类，包括三地址码(Three Address Code, TAC)，静态单赋值形式(Static Single Assignment Form, SSA)，基于栈的IR，等等

在后面的lab中，会逐步增加IR的指令。区分物理栈和运算栈（但是物理上是一个栈）

#### 实验记录

按照指导手册给出的规范，antlr编译时会提示`warning(125): minidecaf.g4:22:6: implicit definition of token Integer in parser`

### step2 常量表达式

### step3 常量表达式

优先级和结合性

### step4 常量表达式

### step5 变量和语句

为了加入变量，我们需要确定：变量存放在哪里、如何访问。 为此，我们会引入栈帧的概念，并介绍它的布局。

但除了把 IR 一条一条替换成汇编，step5 还需要生成 prologue 和 epilogue，并且 ret 也要修改了， 参见栈帧文档。

变量声明

变量信息的维护可以在 IR 生成中进行，也可以在 parser 的同时进行（本文中就是这样做的），也可以专门作为一个步骤进行。在处理变量声明时需要维护已声明变量的信息(add_local())，可以使用链表，也可以使用你喜欢的数据结构来维护它。不过要注意 add_local()中应该同时完成 offset 的确定和变量重复定义的检查。在变量被引用时，你需要通过名称找到对应的结构体。

### step6 变量和语句

悬吊 else 问题

### step7 块语句和作用域和更多语句

不同变量可以重名了。 重名的情况包括作用域内部声明覆盖（shadowing）外部声明，以及不相交的作用域之间的重名变量。 因此，变量名不能唯一标识变量了，同一个变量名 a 出现在代码不同地方可能标识完全不同的变量。 我们需要进行 名称解析（name resolution），确定 AST 中出现的每个变量名分别对应那个变量。

step7 我们需要给自己的编译器新增一个阶段：名称解析，它位于语法分析和 IR 生成之间。名称解析的阶段任务就是把 AST 中出现的每个变量名关联到对应的变量，它需要遍历 AST，所以实现为 AST 上的一个 Visitor，它的输出 是上面那棵 AST，但 AST 中所有涉及变量名的结点都增加一个属性，表示此处的变量名到底标识哪个变量.这样的结点有：primary、assignment 和 declaration。代码中，这样的属性可以实现为指向 变量数据结构（step5栈帧 变量声明）的一个指针。 也可以实现为一个从 AST 结点到变量的映射。

用于储存变量信息的 符号表 的结构也需要改进，以支持作用域。为此，我们把符号表改造为一个栈。

### step8 块语句和作用域和更多语句

我们需要确定：每个 break 和 continue 跳转到的标号是哪个。 实现很容易，类似符号表栈维护 break 标号栈和 continue 标号栈。

### step9 函数和全局变量

类似符号表，我们需要一张表维护函数的信息。 当然，函数不会重名，所以不用解析名称。 这张表主要目的是记录函数本身的信息，方便语义检查。

函数调用约定

### step10 函数和全局变量

全局变量和局部变量不同，它不是分配在栈上，而是放在某个固定地址，写在汇编的 .bss 段或 .data 段里。 访问它也不能通过 fp 加偏移量，而是需要通过它的符号加载它的地址，通过它的地址访问它。

和局部变量一样，全局变量要放进符号表里，名称解析才能解析到它们。 和 step7 一样，符号表是一个栈，其中每个元素对应一个作用域。 全局变量就放在栈底，它们位于全局作用域；名称分析遍历 AST 过程中，栈底元素一直都在，不会被弹出。

### step11 指针和数组

step11 相当复杂，需要我们引入类型系统、左值的概念，并且加入类型检查以及一大堆语义检查。

step11 开始，要增加一个新阶段：类型检查（type checking）；它在名称解析和 IR 生成之间。

左值

类型检查

### step12 指针和数组