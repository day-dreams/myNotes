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

class Top
{
  public:
    int x;

  private:
};

class Left : public Top
{
  public:
    int y;
};

class Right : public Top
{
  public:
    int z;

  private:
};

class Buttom : public Left, public Right
{
  public:
    int xx;

  private:
};

int main(int argc, char** argv)
{
    {
        Son* son = new Son();

        son->print();  // son

        Parent* pa = dynamic_cast<Son*>(son);
        pa->print();  // parent

        delete son;
    }

    {
        Buttom* b = new Buttom();
        Buttom* x = nullptr;

        Right* pa = dynamic_cast<Buttom*>(x);  // it fails
        std::cout << pa << std::endl;

        delete b;
    }
    return 0;
}