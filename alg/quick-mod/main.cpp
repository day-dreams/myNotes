#include <iostream>

/* a^b mod m ,不考虑溢出等情况*/
int quick_mod(int a, int b, int m)
{
    int rv = 1;

    int base = a;

    while (b > 0)
    {
        if (b & 0x1 == 1)
        {
            rv = rv * base % m;
        }

        b >>= 1;

        base = base * base % m;
    }

    return rv;
}

/* 网上找的一个例子 */
int PowerMod(int a, int b, int c)
{
    int ans = 1;
    a       = a % c;
    while (b > 0)
    {
        if (b % 2 == 1)
            ans = (ans * a) % c;
        b = b / 2;
        a = (a * a) % c;
    }
    return ans;
}

int main(int argc, char** argv)
{
    std::cout << quick_mod(2, 1, 3) << ' ' << PowerMod(2, 1, 3) << std::endl;
    std::cout << quick_mod(2, 3, 3) << ' ' << PowerMod(2, 3, 3) << std::endl;
    std::cout << quick_mod(2, 4, 3) << ' ' << PowerMod(2, 4, 3) << std::endl;
    std::cout << quick_mod(2, 5, 3) << ' ' << PowerMod(2, 5, 3) << std::endl;
    std::cout << quick_mod(2, 6, 3) << ' ' << PowerMod(2, 6, 3) << std::endl;
    std::cout << quick_mod(2, 7, 3) << ' ' << PowerMod(2, 7, 3) << std::endl;
    std::cout << quick_mod(3, 4, 5) << ' ' << PowerMod(3, 4, 5) << std::endl;
    std::cout << quick_mod(5, 6, 7) << ' ' << PowerMod(5, 6, 7) << std::endl;
    std::cout << quick_mod(7, 8, 9) << ' ' << PowerMod(7, 8, 9) << std::endl;

    return 0;
}