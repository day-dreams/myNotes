#include <iostream>

class Father
{
  public:
    int x = 1;
    int y = 2;
};

class Son : public Father
{
  public:
    int z = 3;
};

int main(int argc, char** argv)
{
    Son son;

    int* ptr = (int*)(&son);

    std::cout << *ptr++ << std::endl;
    std::cout << *ptr++ << std::endl;
    std::cout << *ptr << std::endl;

    std::cin.get();

    return 0;
}