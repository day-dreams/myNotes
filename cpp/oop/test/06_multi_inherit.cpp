#include <iostream>

class B1
{
  public:
    int x = 1;

    virtual void print1()
    {
        std::cout << "print from B1" << std::endl;
    }

  private:
};

class B2
{
  public:
    int          y = 2;
    virtual void print2()
    {
        std::cout << "print from B2" << std::endl;
    }

  private:
};

class Drive : public B1, public B2
{
  public:
    int          z = 3;
    virtual void print3()
    {
        std::cout << "print from Drive" << std::endl;
    }
};

int main(int argc, char** argv)
{
    typedef void (*VFUN_TYPE)(void);

    Drive d;

    // vptr,8,including 2 fun
    {
        int* vtable = *(int**)(&d);
        VFUN_TYPE (*(vtable + 0))();
        VFUN_TYPE (*(vtable + 2))();
    }

    // x,4
    std::cout << *((int*)(&d) + 2) << std::endl;

    // align,4

    // vptr,8,including 1 fun
    {
        int* vtable = *((int**)(&d) + 2);
        VFUN_TYPE (*(vtable + 0))();
    }

    // y,4
    std::cout << *((int*)(&d) + 6) << std::endl;

    // x,4
    std::cout << *((int*)(&d) + 7) << std::endl;

    std::cout << "size of d:" << sizeof(d) << std::endl;
    std::cin.get();
    return 0;
}