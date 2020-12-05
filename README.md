2020秋编译原理实验（与往年的Decaf没有关系）

实践才能真正理解，也能解决很多疑惑 Get Hands Dirty

增量式，循序渐进

先看到源码熟悉大致框架，再看相关视频和文档会有事半功倍的效果

## LLVM IR

IRVisitor是LLVM IR相关visitor。需要开一个feature branch

TODO 加入LLVM IR的过程挺麻烦的，我还不熟悉llvm ir的使用，感觉文档也不太清晰，只能照着这个tutorial来改，对minidecaf的所有step支持非常不全。更大的问题是，在用clang++编译的时候，会报告antlr的库文件中有错误，根本无法成功编译。

> [My First Language Frontend with LLVM Tutoria(https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html

## 运行

按照实验手册安装Antlr，无需置环境变量，但是需要将Antlr包放到/usr/local/lib

按照官方Cpp runtime构建方法编译，并将include和lib复制到/usr/local/include和/usr/local/lib中（不用运行`ldconfig`）

[官方安装方法](https://github.com/antlr/antlr4/tree/master/runtime/Cpp)

[InstallAntlr4Cpp](http://www.cs.sjsu.edu/~mak/tutorials/InstallANTLR4Cpp.pdf)多了将库复制到/usr/local/include和/usr/local/lib的步骤，最后还要运行`sudo ldconfig`

同时提供Makefile和CMakeList.txt。主要是做了这么多PA，还没有自己从头开始写的作业，我也还没正真写过Makefile，就当作练习。同时，加入LLVM IR之后会用到clang++，感觉用CMake应该会更方便一点。CMake的tutorial看的有点昏，不知道讲什么（我自己没有太多项目经验，需求太少），make的文档也很全，但是我觉得我更需要的是那种Makefile C/Cpp best practice的东西。

riscv toolchain可以到SiFive上下载

```
riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -O3

qemu-riscv32

export CLASSPATH=".:/usr/local/lib/antlr-4.8-complete.jar:$CLASSPATH"
```

### step10 函数和全局变量 全局变量

全局变量和局部变量不同，它不是分配在栈上，而是放在某个固定地址，写在汇编的 .bss 段或 .data 段里。 访问它也不能通过 fp 加偏移量，而是需要通过它的符号加载它的地址，通过它的地址访问它。

和局部变量一样，全局变量要放进**符号表**里，名称解析才能解析到它们。 和 step7 一样，**符号表**是一个栈，其中每个元素对应一个作用域。 全局变量就放在栈底，它们位于全局作用域；名称分析遍历 AST 过程中，栈底元素一直都在，不会被弹出。

局部变量需要通过 frameaddr 访问，但全局变量不行，所以我们引入新的 IR 指令用于加载全局变量的地址。globaladdr	一个字符串，表示符号名	取得符号名对应的全局变量的地址，压入栈中

并且现在，一个 MiniDecaf 程序的 IR 除了一系列 IR 函数和函数中的IR指令，还要包含一系列 IR 全局变量 了，每个需要记录的信息类似：大小有多少字节，是否有初始值，初始值是多少

每个全局变量还对应一段汇编，不过这段汇编基本就是一个模板替换，我们直接给出结果。例如 int compiler = 2020; 放到 .data，其汇编如下，compiler 和 2020 可替换成其他变量名和初始值：
```
	.data
	.globl compiler
	.align 4
	.size compiler, 4
compiler:
	.word 2020
```
而 int tsinghua; 放到 .bss 的汇编如下，第一个 4 表示大小，第二个 4 表示对齐要求
```
.comm	tsinghua,4,4
```
我们实验中，全局变量相对简单。 但其实全局变量可以展开讲到 linker 和 loader，可惜我们课容量有限不能讨论。

### step11 指针和数组 指针

step11 相当复杂，需要我们引入类型系统、左值的概念，并且加入类型检查以及一大堆语义检查。

你需要：1. 首先实现左值分析（无指针的），并通过之前所有测例。2. 然后搭建类型检查的框架（无指针的），并通过之前所有测例。3. 最后，加入指针，并且适当修改你的左值分析和类型检查。完整支持本节引入的所有新特性，通过相关测试

step11 引入解引用操作符以后，左值的概念更加复杂，因此需要增加 左值分析（这个名字是我们自己取的）， 排除 1+2=3 或 &(1+2) 这种代码，并且为后续阶段生成左值地址提供信息。左值分析可以放到名称解析中。

step11 开始，要增加一个新阶段：**类型检查（type checking）**；它在名称解析和 IR 生成之间。 它用于（一）完成和表达式类型相关的一大类语义检查，例如 11.4 和 11.5；（二）计算表达式的类型信息，提供给后续阶段使用。 这一大类的语义检查互相联系很紧密，所以它们被拿出来单独作为一个阶段。

类型检查阶段是由一系列 类型规则 指导进行的，源代码中操作如果需要类型检查，那它需要先声明自己的类型规则。 对于某个操作，其类型规则包含两个部分：1. 各个源操作数的类型有什么限制；2. 如果源操作数的类型满足 1.，那么操作的结果（如果有）是什么类型。

类型检查阶段完成后，为了把类型信息传递给后续阶段，AST 中每个表达式结点都要新增一个属性 type 描述它的类型。 

无须新增 IR 指令，但是要用现有指令来支持新语言特性。如解引用 * 的 IR 就是子表达式的 IR 接上一个 load。

生成代码的过程中，对于左值要生成它们的地址，右值要生成它们的值。一个表达式（例如 a）可能有时是左值有时不是，取决于它出现在哪儿。

[手写 parser 简单实例](https://github.com/decaf-lang/minidecaf-tutorial/blob/master/docs/lab11/manual-parser.md)，包含类型系统、左值计算、强制类型转换的实现

### step12 指针和数组 数组

引入数组后，变量的大小不一定是 4 了，因此变量的数据结构还需要增加一个 size 属性，并且变量的 frameaddr 不一定连续了（但每个变量所占的一片内存空间一定连续）。

本质上，int a; 的 a 的值被存放在内存中，需要一次访存 load 它的frameaddr 才能取得。 而局部变量 int a[2]; 中的 a 是一个编译期就确定的偏移量常数，它的值就是 fp 加上这个偏移量常数，无须访存。