#include <iostream>

int count = 0;

bool is_triangle(int& a, int& b, int& c)
{
    if (a == 0 || b == 0 || c == 0)
        return false;

    if (a + b > c && b + c > a && a + c > b)
        return true;

    return false;
}

bool is_dengyao(int& a, int& b, int& c)
{
    if (a == b)
        return true;
    if (b == c)
        return true;
    if (c == a)
        return true;

    return false;
}

int main(int argc, char** argv)
{
    int i = 100;

    for (; i <= 999; ++i)
    {
        int a = i % 10;
        int b = i / 10 % 10;
        int c = i / 100;
        if (is_triangle(a, b, c) && is_dengyao(a, b, c))
        {
            ++count;
        }
    }

    std::cout << count << std::endl;
    return 0;
}