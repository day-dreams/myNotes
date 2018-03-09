#include <iostream>

/* 快速幂n^m,不考虑溢出问题 */
int quick_power(int n, int m)
{
    int rv = 1;

    int base = n;

    while (m > 0)
    {
        if (m & 0x1 == 1)
        {
            rv *= base;
        }

        base *= base;
        m >>= 1;
    }

    return rv;
}

int main(int argc, char** argv)
{
    std::cout << quick_power(1, 3) << std::endl;
    std::cout << quick_power(2, 3) << std::endl;
    std::cout << quick_power(3, 3) << std::endl;
    std::cout << quick_power(4, 5) << std::endl;
    std::cout << quick_power(6, 9) << std::endl;
    std::cout << quick_power(9, 9) << std::endl;

    std::cin.get();

    return 0;
}