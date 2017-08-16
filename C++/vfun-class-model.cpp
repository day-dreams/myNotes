/**
 * @author daydream
 * @email [zn.moon2016@gmail.com]
 * @create date 2017-06-18 08:30:24
 * @modify date 2017-06-18 08:30:24
 * @desc [待虚函数的对象内存布局]
*/

#include <iostream>

using namespace std;

class Base {
  int64_t a = -1;
  int64_t b = -2;
  virtual void vfun_1() { cout << "virtual fun.1 in Base" << endl; }
  virtual void vfun_2() { cout << "virtual fun.2 in Base" << endl; }
};

class Wrapper : Base {
  int64_t c = -3;
  int64_t d = -4;
  virtual void vfun_1() { cout << "virtual fun.1 in Wrapper" << endl; }
  virtual void vfun_2() { cout << "virtual fun.2 in Wrapper" << endl; }
};

class MoreWrapper : Wrapper {
  int64_t e = -5;
  int64_t f = -6;
  virtual void vfun_1() { cout << "virtual fun.1 in DeepWrapper" << endl; }
  virtual void vfun_2() { cout << "virtual fun.2 in DeepWrapper" << endl; }
};

typedef void (*pfun)(); //函数指针

int main(int argc, char **argv) {
  //   Base object;
  //   Wrapper object;
  MoreWrapper object;

  {
    auto ptr = (int64_t *)(&object); // 4字节,因为成员变量int是4字节
    cout << "address of object:" << &object << endl;
    cout << "size of ptr:" << sizeof(ptr) << endl;
    cout << "vptr = " << (ptr) << endl;
    cout << "a = " << *(ptr + 1) << ' ' << ptr + 1
         << endl; // warning:仅供学习,不建议实践中使用
    cout << "b = " << *(ptr + 2) << ' ' << ptr + 2
         << endl; // warning:仅供学习,不建议实践中使用
    cout << "c = " << *(ptr + 3) << ' ' << ptr + 3
         << endl; // warning:仅供学习,不建议实践中使用
    cout << "d = " << *(ptr + 4) << ' ' << ptr + 4
         << endl; // warning:仅供学习,不建议实践中使用
  }
  {
    auto ptr = (void *)(&object);  //对象的首地址
    auto ptr_vt = *(int64_t *)ptr; //对象的虚函数表首地址指针

    for (int i = 0; i < 2; ++i) {
      auto p = (int64_t *)ptr_vt + i;
      cout << p << " ";
      auto fun = (pfun)*p;
      fun();
    }

    for (int i = 0; i < 10; ++i) { //输出虚函数表的内容,试图观察表尾,失败
      auto p = (int64_t *)ptr_vt + i;
      cout << "_end of vtable: " << p << " " << *p << endl;
    }
  }
  return 0;
}
