#pragma once
#include <string>
#include <vector>

struct Var {
  int index; // start with 1
  int init;  // for global variable
  int type; // 甚至你可以就用整数表示类型,array -1, int 0,*1,**2...
  // step 12
  int basetype;
  int size;
  std::vector<int> length_list;
};

struct Func {
  // step11 开始还需要记录参数和返回值类型
  int paranum;
  int framesize;
  int type; // 表示返回值类型，甚至你可以就用整数表示类型,int 0,*1,**2...
  std::vector<std::pair<int, std::string>> parameter_list;
};

struct NodeInfo {
  int type; // 甚至你可以就用整数表示类型,int 0,*1,**2...
  bool is_lvalue;
  int basetype; // for array
};

void ERROR(std::string error_message) {
    std::cout << "[ERROR] " << error_message;
    exit(1);
}
