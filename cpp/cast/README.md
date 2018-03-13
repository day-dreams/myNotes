C++中四种类型转换
===

<!-- TOC -->

- [static_cast](#static_cast)
- [reinterpret_cast](#reinterpret_cast)
- [const_cast](#const_cast)
	- [使用范例](#使用范例)
- [dynamic_cast](#dynamic_cast)

<!-- /TOC -->

## static_cast

就是C语言里的强制转换,不同的是,static_cast运行期完成的,编译器会安插代码,以完成正确的转换.

static_cast不保证精度问题.


## reinterpret_cast

有点像static_cast,但reinterpret_cast实在编译期完成的转化,并不保证符合继承关系,而是直接在bit级别上,弄出一个有相同地址但可能不同类型的对象.

它相对static_cast的有点可能在于提高了效率吧,但我觉得一点卵用都没有.如果你的设计里需要频繁的各种cast,一定是设计有问题.

## const_cast

const_cast只用于去掉/增加引用、指针的顶层const属性。

在const_cast之后，随便怎么修改都可以。只有一个情况例外：如果原来的对象是指向const对象（底层const）的const指针/引用（顶层const），const_cast虽然能去掉顶层const，也能够达到去底层const的效果，但是任何对被引用/指向的对象的修改都是为定义行为。比如：
```cpp
const int x=1;
const int * const ptr1=&x;
int * ptr2=const_cast<int*>(ptr1);//ok

*ptr2=2;//ok,可以编译通过，但是为定义行为，可能不会对x产生修改

ptr=nullptr;//ok
```

### 使用范例

这个的用途主要是在函数重载里，举个例子：

```cpp
const int& max(const int & x,const int &y){
	return x>y?x:y;
}
```

如果这样用,就会报错:
```cpp
int x = 1, y = 2;
// int&       maxone = max(x, y);  // error
const int& maxone = max(x, y);  // ok
// maxone            = 10;//error
```


但如果我们一定要对非const的x和y比较,并改变较大的那个的值呢?这时就可以使用const_cast了:
```cpp
int& max(int& x, int& y)
{
    // error!会引起无限的递归调用
    // auto& r = max(const_cast<int&>(x), const_cast<int&>(y));  //

    auto& r = max(const_cast<const int&>(x), const_cast<const int&>(y));

    return const_cast<int&>(r);
}
```

## dynamic_cast

cppreference是这样说的:

>Safely converts pointers and references to classes up, down, and sideways along the inheritance hierarchy.

安全的把指向class的指针或引用,向**上转化为父类指针/引用**或**向下转化为子类指针/引用**.

那么什么时候是不安全的呢?比如原来的指针是个空指针,那么一定是不能转化的.

一般用这个,就是为了调用父/子类的非虚函数.本来不应该出现这样的东西,但有时侯可能就是无法避免,比如因为性能原因不能定义虚函数.

```cpp
#include <iostream>

class Parent
{
  public:
    void print()
    {
        std::cout << "parent" << std::endl;
    }

  private:
};

class Son : public Parent
{
  public:
    void print()
    {
        std::cout << "son" << std::endl;
    }

  private:
};

int main(int argc, char** argv)
{
    Son* son = new Son();

    son->print();  // son

    Parent* pa = dynamic_cast<Son*>(son);
    pa->print();  // parent

    delete son;
    return 0;
}
```

[这里](test/test_dynamic_cast.cpp)有使用例子.


