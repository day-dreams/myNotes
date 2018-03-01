#include <iostream>
#include <vector>

void func(char x[5])
{
    std::cout << sizeof(x) << std::endl;
}

void foo()
{
    std::cout << sizeof(malloc(sizeof(int) * 10)) << std::endl;
}

void foo1(std::vector<int> x)
{
    std::cout << sizeof(x) << std::endl;
}

int main(int argc, char** argv)
{
    {
        std::cout << "case 1:" << std::endl;
        char x[32] = "123";
        std::cout << sizeof(x) << std::endl;
    }
    {
        std::cout << "case 2:" << std::endl;

        char  x[32] = "123";
        char* y     = x;
        std::cout << sizeof(y) << std::endl;
    }
    {
        std::cout << "case 3:" << std::endl;

        int i = 0;

        std::cout << i << std::endl;
        std::cout << sizeof(++i) << std::endl;
        std::cout << i << std::endl;
    }
    {
        std::cout << "case 4:" << std::endl;

        char* x;
        func(x);
        char j[5];
        func(j);
    }
    {
        std::cout << "case 5:" << std::endl;
        int i[32];
        std::cout << sizeof(i) << std::endl;
    }
    {
        std::cout << "case 6:" << std::endl;
        int  i[] = {1, 2, 3, 4, 5};
        int* j   = i;
        std::cout << sizeof(i) << std::endl;
        std::cout << sizeof(j) << std::endl;
    }

    {
        std::cout << "case 7:" << std::endl;
        foo();
    }
    {
        std::cout << "case 8:" << std::endl;
        foo1(std::vector<int>());
    }
    return 0;
}