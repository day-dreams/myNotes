#include <iostream>

class A
{
  public:
    virtual void A1()
    {
        std::cout << "from A1" << std::endl;
    }
    virtual void A2()
    {
        std::cout << "from A2" << std::endl;
    }
    virtual void A3()
    {
        std::cout << "from A3" << std::endl;
    }

  protected:
    int a1;
    int a2;
};

int main(int argc, char** argv)
{
    A x;

    typedef void (*VFUN_TYPE)(void);

    // vtable指向虚函数表
    int* vtable = *(int**)(&x);

    VFUN_TYPE (*(vtable + 0))();
    VFUN_TYPE (*(vtable + 2))();
    VFUN_TYPE (*(vtable + 4))();

    std::cout << "sizeof x:" << sizeof(x) << std::endl;

    std::cin.get();

    return 0;
}