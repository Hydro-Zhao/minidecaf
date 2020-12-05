2020秋编译原理实验（与往年的Decaf没有关系）

总体流程 lexer -> parser -> 名称解析（name resolution） -> 类型检查（type checking） -> 生成IR -> 生成汇编 （实验的主要部分就是semantic analysis）

实践才能真正理解，也能解决很多疑惑 Get Hands Dirty

增量式，循序渐进

先看到源码熟悉大致框架，再看相关视频和文档会有事半功倍的效果

## 

IRVisitor是LLVM IR相关visitor。需要开一个feature branch

## LLVM IR

TODO 加入LLVM IR的过程挺麻烦的，我还不熟悉llvm ir的使用，感觉文档也不太清晰，只能照着这个tutorial来改，对minidecaf的所有step支持非常不全。更大的问题是，在用clang++编译的时候，会报告antlr的库文件中有错误，根本无法成功编译。

> [My First Language Frontend with LLVM Tutoria(https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html

## 运行

按照实验手册安装Antlr，无需置环境变量，但是需要将Antlr包放到/usr/local/lib

按照官方Cpp runtime构建方法编译，并将include和lib复制到/usr/local/include和/usr/local/lib中（不用运行`ldconfig`）

[官方安装方法](https://github.com/antlr/antlr4/tree/master/runtime/Cpp)

[InstallAntlr4Cpp](http://www.cs.sjsu.edu/~mak/tutorials/InstallANTLR4Cpp.pdf)多了将库复制到/usr/local/include和/usr/local/lib的步骤，最后还要运行`sudo ldconfig`

同时提供Makefile和CMakeList.txt。主要是做了这么多PA，还没有自己从头开始写的作业，我也还没正真写过Makefile，就当作练习。同时，加入LLVM IR之后会用到clang++，感觉用CMake应该会更方便一点。CMake的tutorial看的有点昏，不知道讲什么（我自己没有太多项目经验，需求太少），make的文档也很全，但是我觉得我更需要的是那种Makefile C/Cpp best practice的东西。

## 文档摘要及实验记录

> [minidecaf-tutorial](https://github.com/decaf-lang/minidecaf-tutorial)

### step0

riscv toolchain可以到SiFive上下载

```
riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -O3

qemu-riscv32

export CLASSPATH=".:/usr/local/lib/antlr-4.8-complete.jar:$CLASSPATH"
```

### step1 第一个编译器 仅一个return的main函数

如果表示整数的字符串本身超过INT_MAX，stoi还有用吗？正确的做法应该是比较字符串，思维不要太僵硬。

要先熟悉Antlr的使用（包括生成的cpp，header文件提供的API），了解生成的cpp、header文件与lexer.g4和parser.g4的关系。

[Vistor模式](https://github.com/decaf-lang/minidecaf-tutorial/blob/master/docs/lab1/visitor.md)

Antlr**规则命名**。listener, visitor, 产生式动作

visit函数返回值的类型是antlrcpp::Any，

> https://tomassetti.me/getting-started-antlr-cpp/
> There is only one small thing to consider when using the default visitor: it returns a antlrcpp::Any object. This is a simple structure, used to accept any type. So, it does not impose any practical burden, but it is something to remember.

context 就是 AST 的结点，每个 context 也有一个 accept 函数接受 visitor

visitor 还自带一个方法 visitChildren：遍历所有子结点，返回最后一个子结点的返回值。但是任然有访问单个context的visit函数？）

### step2 常量表达式 一元操作

你可以把三条指令变成一条 Unary(op)，其中 op 是 "-"、"~" 或 "!"（即把这三个op写在一行，再用Antlr规则命名命名为Unary，在visitUnary中对不同操作进行处理）

遍历 AST 时，遇到一元表达式的时候，先生成子表达式的 IR，然后再根据操作类型生成一个 neg 或 not 或 lnot

### step3 常量表达式 加减乘除模

### step4 常量表达式 比较和逻辑表达式

### step5 变量和语句 局部变量和赋值

对每个变量用一个数据结构维护其信息，如**名称、类型（step11）、声明位置、初始值等**。这个保存变量符号的表被称为**符号表（symbol table）**。那**变量偏移量**可以（一）作为变量数据结构的一个字段、（二）也可以维护一个变量到偏移量的映射、（三）通过某种方法计算得到。

当然，不能用一张符号表覆盖整个程序，程序中不同位置的符号表是不同的。 例如，符号表只会包含被声明的变量的信息，因此在 int a=0; 之后的符号表比之前的多了一个条目表示 a 对应的变量数据结构。

目前我们没有作用域的概念，在第7章引入作用域后，可以以节省空间的方法保存局部变量。

引入栈帧的概念。

区分IR物理栈和运算栈，但是我们之前一直在使用物理栈的空间来实现运算栈

遇到读取变量 primary: Identifier 的时候，查符号表确定变量是第几个，然后生成 **frameaddr（生成栈上地址）**和 load。

**FRAMESIZE** 是一个编译期已知的常量，等于 8 + 4 * 局部变量个数

为了计算 prologue 中分配栈帧的大小，IR 除了一个指令列表，还要包含一个信息：局部变量的个数。

step5 还需要生成 prologue 和 epilogue。

### step6 变量和语句 if语句和条件表达式

注意一个程序中的标号，也就是 label 的参数，必须唯一，否则跳转目的地就不唯一了。 简单地维护一个计数器即可，例如 label l1, label l2, label l3 ...

### step7 块语句和作用域和更多语句 作用域和块语句

step7 我们需要给自己的编译器新增一个阶段：**名称解析（name resolution）**，确定 AST 中出现的每个变量名分别对应那个变量。它位于语法分析和 IR 生成之间。它的输出是上面那棵 AST，但 AST 中所有涉及变量名的结点都增加一个属性，表示此处的变量名到底标识哪个变量。代码中，这样的属性可以实现为指向变量数据结构（step5栈帧 变量声明）的一个指针，也可以实现为一个从 AST 结点到变量的映射。因为变量相关的语义检查和名称解析密切相关，所以可以放到那里面去

用于储存变量信息的**符号表的结构**也需要改进，以支持作用域。具体的，它需要支持1.符号表中，区分不同作用域的变量：支持声明覆盖（shadowing)、检查重复声明;2.离开某作用域时，要把其中的变量从符号表中删除。为此，我们把符号表改造为一个栈。栈中每个元素都对应一个开作用域，是一个列表，包含该作用域中至今声明过的所有变量。每进入一个作用域，就压栈一个空列表；离开作用域就弹栈。在符号表中查找变量名，从栈顶往下查找。每次遇到变量名时查找符号表，将其关联到具体的变量，或者报错变量未声明；每次遇到声明，确定 frameaddr、建立变量并插入符号表，或者报错变量重复声明。

另外， 变量偏移量 和 栈帧大小 的计算方法可以做出修改。确定不能同时有用的变量可以使用同一片物理空间来保存，这样可以节约空间。如果我们还假设偏移量是 -12-4*frameaddr，那变量的 frameaddr 意义需要变化。 为了保证 step5 中叙述的栈帧性质，变量 frameaddr 的含义要改为是 “在此变量刚声明之前，所有开作用域中的变量总数”。

### step8 块语句和作用域和更多语句 循环语句

我们需要确定每个 break 和 continue 跳转到的标号是哪个。 实现很容易，类似符号表栈维护 break 标号栈和 continue 标号栈。遇到 Loop(...) 就（一）创建这个循环的 break 标号和 continue 标号（以及起始标号）； （二）把两个标号压入各自栈里； （三）离开 Loop 的时候弹栈。和 step6 一样，各个循环的标号需要唯一，简单地后缀一个计数器即可。每次遇到 break 语句，其跳转目标就是 break 标号栈的栈顶，如果栈为空就报错。continue 语句类似。

### step9 函数和全局变量 函数

类似符号表，我们需要**一张表维护函数的信息**。 当然，函数不会重名，所以不用解析名称。 这张表主要目的是记录函数本身的信息，方便语义检查。就 step9 而言，这个信息包括：参数个数（step11 开始还需要记录参数和返回值类型）。

现在栈帧包含四块，从顶向下依次是运算栈、实参、局部变量、fp 和 ra

还有一个问题就是形参的处理，例如foo()中调用bar()，bar 要访问参数a,b，那a,b放在哪儿？ 可以直接使用 foo 栈帧上的实参，那么 a 相对 fp 的偏移量为 0，同理 b 偏移量为 4。 因此 step7 中的偏移量计算方法仅限非参数的局部变量，而第 k>=0 个参数相对于 fp 的偏移量是 4*k。还有一种方法是把参数当成普通局部变量，在 prologue 中复制到栈帧中局部变量的区域（即两种的区别是callee的参数是在caller的栈帧上还是callee的栈帧上）。

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