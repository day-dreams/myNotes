# push_back VS emplace_back

emplace_back一般比push_back高效,特别是在没有移动构造函数的情况下.

## 两类插入函数

STL容器有这两类插入函数:  
* emplace,emplace_back,emplace_front
* insert,push_back,push_front

第一类执行的操作是"直接放置".他们接受不定长的参数列表,在内部使用std::bind()来绑定对应的构造函数.

第二类是"复制再插入".他们只可能调用copy或者move构造函数.

## 二者在实践上的差别

当我们要凭空向容器凭空插入一个元素的时候,有两种选择.
* 先使用默认构造函数构造一个对象,再把对象传入第二类函数来实现复制/移动
* 直接把构造对象需要的参数,传入第一类函数,直接调用默认的构造函数.

第二种方法只用调用一次构造函数,相对比较高效.如果对象构造的代价很大,显然我们更倾向于第二种插入方法.

[这里](test.cpp)有实例代码.

```c++
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

  // emplace_back 可以直接调用默认的构造函数
  col.emplace_back(10);         // Object(x)
  col.emplace_back(10,20);      //Object(x,y)
  return 0;
}
```