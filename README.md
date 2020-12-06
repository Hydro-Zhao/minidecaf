2020秋编译原理实验（与往年的Decaf没有关系）

完成了step1-12的名称解析，类型检查，中间代码生成

TODO 后续还要生成汇编。

Get Hands Dirty

增量式，循序渐进

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