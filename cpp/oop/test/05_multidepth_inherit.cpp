#include <iostream>

class Grandpa
{
  public:
    int x = 1;
};

class Parent : public Grandpa
{
  public:
    int y = 2;

  private:
};

class Son : public Parent
{
  public:
    int z = 3;
};

int main(int argc, char** argv)
{
    Son son;

    std::cout << *((int*)(&son) + 0) << std::endl;
    std::cout << *((int*)(&son) + 1) << std::endl;
    std::cout << *((int*)(&son) + 2) << std::endl;

    std::cin.get();
    return 0;
}