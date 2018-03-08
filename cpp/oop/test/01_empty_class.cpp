#include <iostream>

class Empty
{
};

int main(int argc, char** argv)
{
    std::cout << "sizeof empty: " << sizeof(Empty) << std::endl;

    std::cin.get();

    return 0;
}