#include <iostream>

using namespace std;

class Base
{
  public:
    int x = 1;

    void print()
    {
        std::cout << "Base" << std::endl;
    }
    virtual void show()
    {
        std::cout << "show from Base" << std::endl;
    }
};

class Son : public Base
{
  public:
    int x = 2;

    void print()
    {
        std::cout << "Son" << std::endl;
    }
    virtual void show()
    {
        std::cout << "show from Son" << std::endl;
    }
};

int main(int argc, char** argv)
{
    Base* x = new Son();
    std::cout << x->x << std::endl;

    x->print();
    x->show();

    Son* y = (Son*)x;
    std::cout << y->x << std::endl;

    cin.get();
    return 0;
}