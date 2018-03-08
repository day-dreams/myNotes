#include <iostream>

class A
{
  public:
    int          x = 1;
    virtual void print()
    {
        std::cout << "A::print()" << std::endl;
    }

  private:
};

class B : public A
{
  public:
    int y = 1;
    // virtual void print()
    // {
    //     std::cout << "B::print()" << std::endl;
    // }
    virtual void B_print()
    {
        std::cout << "B_print()" << std::endl;
    }

  private:
};

int main(int argc, char** argv)
{
    typedef void (*VFUN_TYPE)(void);

    B x;

    int* vtable = *((int**)&x);

    VFUN_TYPE (*(vtable + 0))();
    VFUN_TYPE (*(vtable + 2))();
    // VFUN_TYPE (*(vtable + 0))();

    std::cout << "sizeof B:" << sizeof(x) << std::endl;

    std::cin.get();

    return 0;
}