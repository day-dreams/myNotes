#include <iostream>

const int& max(const int& x, const int& y)
{
    return x > y ? x : y;
}

int& max(int& x, int& y)
{
    // error!会引起无限的递归调用
    // auto& r = max(const_cast<int&>(x), const_cast<int&>(y));  //

    auto& r = max(const_cast<const int&>(x), const_cast<const int&>(y));

    return const_cast<int&>(r);
}

int main(int argc, char** argv)
{
    {
        const int        x    = 10;
        const int* const p    = &x; /* 顶层const */
        const int&       ref  = x;
        int*             copy = const_cast<int*>(p);

        std::cout << x << std::endl;
        std::cout << copy << std::endl;

        *copy = 100;      // ok,ub
        copy  = nullptr;  // ok

        std::cout << copy << std::endl;
        std::cout << x << std::endl;
    }

    {
        int x = 1, y = 2;
        std::cout << x << ' ' << y << std::endl;
        const int& maxone1 = max(x, y);  // ok
        int&       maxone2 = max(x, y);

        ++maxone2;
        // ++maxone1;  // error

        std::cout << x << ' ' << y << std::endl;

        // maxone            = 10;//error
    }
    return 0;
}