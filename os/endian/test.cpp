#include <iostream>

bool isBigEndian()
{
    int num = 1;

    auto x = (char*)(&num);

    if (*x == 1)
        return false;
    else
        return true;
}

union _endian
{
    int  x;
    char y;
};

bool isLittelEndian()
{
    _endian x;
    x.x = 0x12345678;
    if (x.y == 0x12)
        return false;
    else
        return true;
}

int main(int argc, char** argv)
{
    if (isBigEndian())
        std::cout << "big endian" << std::endl;
    else
        std::cout << "little endian" << std::endl;

    if (isLittelEndian())
        std::cout << "little endian" << std::endl;
    else
        std::cout << "big endian" << std::endl;

    return 0;
}