#include <iostream>
using namespace std;

/*
    compile and run:
        g++ -fno-elide-constructors -std=c++11 test.cpp
        g++ -std=c++11 test.cpp;./a.out

    -fno-elide-constructors: 强制禁用RVO优化
 */

class Object {
public:
  Object() { std::cout << "default constructor" << std::endl; }
  Object(const Object &other) { std::cout << "copy constructor" << std::endl; }
  Object(Object &&other) { std::cout << "move constructor" << std::endl; }
  ~Object() { std::cout << "destuctor" << std::endl; }

private:
};

Object generate_object_a() { return Object(); }

// 触发RVO
Object generate_object_b() {
  Object x;
  return x;
}

//运行时才能确定的分支,不触发RVO
Object generate_object_c(bool flag) {
  Object a, b;
  if (flag)
    return a;
  else
    return b;
}

// 定义的返回值类型(Object)和返回的变量类型(Object&&)不一致,不触发RVO
Object generate_object_d() {
  Object a;
  return std::move(a);
}

// 触发RVO
Object &&generate_object_e() {
  Object a;
  return std::move(a);
}

//编译期能确定的分支,也不触发RVO
Object generate_object_f() {
  Object a, b;
  if (true) {
    return a;
  } else {
    return b;
  }
}
int main(int argc, char **argv) {

  {
    std::cout << std::endl;
    auto x = generate_object_a();
  }
  {
    std::cout << std::endl;
    auto x = generate_object_b();
  }
  {
    std::cout << std::endl;
    auto x = generate_object_c(true);
  }
  {
    std::cout << std::endl;
    auto x = generate_object_d();
  }
  {
    std::cout << std::endl;
    generate_object_e();
  }
  {
    std::cout << std::endl;
    generate_object_f();
  }
  return 0;
}