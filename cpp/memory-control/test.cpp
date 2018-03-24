#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <new>
#include <vector>
using namespace std;

void test_malloc()
{
    auto ptr = malloc(sizeof(int) * 10);
    free(ptr);
}

void test_calloc()
{
    auto ptr = calloc(1, sizeof(int));
    free(ptr);
}

void test_realloc()
{
    auto ptr = malloc(sizeof(int) * 100);
    ptr      = realloc(ptr, 1000);
    free(ptr);
}

void test_operator_new_delete()
{
    {
        auto ptr = operator new(10);
                   operator delete(ptr);
    }
    {
        auto ptr = operator new[](10);
                   operator delete[](ptr);
    }
}

// void *operator new(size_t size)
// {
//     std::cout << "operator new called." << size << std::endl;
//     return malloc(size);
// }
// void operator delete(void *ptr)
// {
//     std::cout << "operator delete called." << std::endl;
//     return free(ptr);
// }
// void *operator new[](size_t size)
// {
//     std::cout << "operator new[] called." << size << std::endl;
//     return malloc(size);
// }
// void operator delete[](void *ptr)
// {
//     std::cout << "operator delete[] called." << std::endl;
//     return free(ptr);
// }

class Object
{
  public:
    Object()
    {
        cout << "constructed" << endl;
    }
    ~Object()
    {
        std::cout << "destructed" << std::endl;
    }

  private:
    int x;
};

void test_expression_new_delete()
{
    {
        auto x = new Object;
        delete x;
    }
    {
        auto x = new Object[10];
        delete[] x;
    }
}

class Int
{
  public:
    int x = 1;
    int y = 2;
    int z = 3;
};

int main(int argc, char **argv)
{
    //   test_malloc();
    //   test_calloc();
    //   test_realloc();
    //   test_operator_new_delete();
    test_expression_new_delete();
    return 0;
}