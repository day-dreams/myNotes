#include <iostream>
#include <list>
#include <vector>
using namespace std;

class Object {
public:
  Object(int x) { std::cout << "default constructor(x)" << std::endl; }
  Object(int x, int y) { std::cout << "default constructor(x,y)" << std::endl; }
  Object(int x, int y, int z) {
    std::cout << "default constructor(x,y,z)" << std::endl;
  }
  Object(Object &&other) { std::cout << "move constructor" << std::endl; }
  Object(const Object &other) { std::cout << "copy constructor" << std::endl; }
  ~Object() { std::cout << "destructor" << std::endl; }

private:
};

int main(int argc, char **argv) {
  list<Object> col;

  // push_back要求有一个对象供我们复制/移动
  Object x(10);
  col.push_back(x);            // Object(const Object &other)
  col.push_back(std::move(x)); // Object(Object &&other)

  // emplace_back是根据参数,选择要调用的构造函数,不一定会发生复制/移动操作
  col.emplace_back(10);         // Object(x)
  col.emplace_back(10, 10);     // Object(x,y)
  col.emplace_back(10, 10, 10); // Object(x,y,z)
  col.emplace_back(Object(10)); // Object(x),Object(Object &&other)
  col.emplace_back(x);          // Object(const Object &other)
  return 0;
}