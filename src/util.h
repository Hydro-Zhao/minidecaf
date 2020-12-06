# pragma once

class Var {
    int index; // start with 1
    int init; // for global variable
    int type;// 甚至你可以就用整数表示类型,int 0,*1,**2...
};

class Func { 
    // step11 开始还需要记录参数和返回值类型
    int paranum;
    int framesize;
    int type; // 表示返回值类型，甚至你可以就用整数表示类型,int 0,*1,**2...
};

class NodeInfo {
    int type;// 甚至你可以就用整数表示类型,int 0,*1,**2...
+	bool is_lvalue;
};
