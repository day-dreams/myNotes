#include <iostream>
#include <stdio.h>
struct x_
{
    char a;  // 1 byte,3 bytes for padding

    int   b;  // 4 bytes
    short c;  // 2 bytes
    char  d;  // 1 byte,1byte for padding
} MyStruct;

struct mystruct1
{
    // sizeof(mystruct1)==24
    char  a;  // 1 bytes, 7 bytes for padding
    char* b;  // 8 bytes
    int   c;  // 4 bytes, 4 bytes for padding
};

struct mystruct2
{
    // sizeof(mystruct2)==16
    char* b;  // 8 bytes
    int   c;  // 4 bytes
    char  a;  // 1 bytes
};
int main(int argc, char** argv)
{
    {
        std::cout << sizeof(x_) << std::endl;
    }

    {
        // 变量有可能被调整布局顺序
        char      a;
        struct x_ object1;
        int       x;
        char      c;
        struct x_ object2;
        printf("add: %p\n", &a);
        printf("add: %p\n", &x);
        printf("add: %p\n", &c);
        printf("add: %p\n", &object1);
        printf("add: %p\n", &object2);
    }

    {
        std::cout << sizeof(mystruct1) << std::endl;
        std::cout << sizeof(mystruct2) << std::endl;
    }

    return 0;
}
