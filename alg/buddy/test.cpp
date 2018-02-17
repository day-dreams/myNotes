#include "buddy.h"
#include <iostream>
// g++ -std=c++11 test.cpp buddy.cpp

int main(int argc, char** argv)
{
    buddySystem x(5);

    x.show();
    std::cin.get();

    auto block1 = x.alloc_pages(1);
    x.show();
    std::cin.get();

    auto block2 = x.alloc_pages(0);
    x.show();
    std::cin.get();

    auto block3 = x.alloc_pages(1);
    x.show();
    std::cin.get();

    auto block4 = x.alloc_pages(1);
    x.show();
    std::cin.get();

    auto block5 = x.alloc_pages(1);
    x.show();
    std::cin.get();

    x.free(block1);
    x.show();
    std::cin.get();

    x.free(block2);
    x.show();
    std::cin.get();

    x.free(block3);
    x.show();
    std::cin.get();

    x.free(block4);
    x.show();
    std::cin.get();

    x.free(block5);
    x.show();
    std::cin.get();

    // error!
    // x.alloc_pages(10);
    return 0;
}