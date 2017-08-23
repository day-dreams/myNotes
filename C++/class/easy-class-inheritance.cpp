/**
 * @author daydream
 * @email [zn.moon2016@gmail.com]
 * @create date 2017-06-18 07:26:20
 * @modify date 2017-06-18 07:26:20
 * @desc [简单类在继承情况下的内存布局]
*/

/*
  测试平台: g++ (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609
  build:  g++ -std=c++11 easy-class-inheritance.cpp ;./a.out
*/

#include <iostream>

class Base {
  int a = 1;
  int b = 2;
};

class Wrapper : Base {
  int c = 3;
  int d = 4;
};

class MoreWrapper : Wrapper {
  int e = 5;
  int f = 6;
};
int main(int argc, char **argv) {
  using namespace std;
  MoreWrapper object;

  auto ptr = (int *)(&object);

  cout << "a = " << *(ptr + 0) << endl; // warning:仅供学习,不建议实践中使用
  cout << "b = " << *(ptr + 1) << endl; // warning:仅供学习,不建议实践中使用
  cout << "c = " << *(ptr + 2) << endl; // warning:仅供学习,不建议实践中使用
  cout << "d = " << *(ptr + 3) << endl; // warning:仅供学习,不建议实践中使用
  cout << "e = " << *(ptr + 4) << endl; // warning:仅供学习,不建议实践中使用
  cout << "f = " << *(ptr + 5) << endl; // warning:仅供学习,不建议实践中使用

  cout << "g = " << *(ptr + 6) << endl; // wrong: 未定义行为
  return 0;
}