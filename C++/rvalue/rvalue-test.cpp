#include <iostream>

/*
    g++ -std=c++11 rvalue-test.cpp;./a.out;
*/

using namespace std;

class Test {
public:
  Test() { cout << "+ init constructor" << endl; }
  Test(Test &other) { cout << "* copy constructor" << endl; }   /* lvalue */
  Test(Test &&other) { cout << "** move constructor" << endl; } /* rvalue */
  // Test(const Test &other) {
  //   cout << "** move/copy constructor" << endl;
  // } /* lvalue and rvalue */
  Test &operator=(Test &&other) { cout << "** move assignment" << endl; }
  Test &operator=(Test &other) { cout << "* copy assignment" << endl; }
  ~Test() { cout << "- destructor" << endl; }
};

// void cap_parameters(Test other) { cout << "captured value" << endl; }
void cap_parameters(Test &other) {
  cout << "captured lvalue reference" << endl;
}
void cap_parameters(Test &&other) {
  cout << "captured rvalue reference" << endl;
}
void cap_parameters(const Test &other) {
  cout << "captured lvalue/rvalue reference" << endl;
}

void fun1() {
  cout << "in fun" << 1 << ":" << endl;
  Test object;
}
Test fun2() {
  cout << "in fun" << 2 << ":" << endl;
  Test object;
  // gcc/clang++下，object不会被复制，而是直接返回这个函数体内的object，避免复制发生
  // 这是一种默认的编译器优化，关闭：-fno-elide-constructors
  return object;
}

// template <class T> void fun3(Test t) {}
// template <class T> void fun4(T &&arg) { fun3(arg); }

int main() {
  Test object;

  /* 下面的不是一个对象构造，编译器认为是一个函数声明，
    函数参数是一个返回Test类型的函数指针，返回值是一个Test对象
  */
  Test a(Test());

  Test b(move(object));

  cout << "=====================\n";

  cap_parameters(object);
  cap_parameters(std::move(object));

  fun1();
  auto c = fun2();

  return 0;
}