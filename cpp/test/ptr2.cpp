#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    int a[5] = {1, 2, 3, 4, 5};

    std::cout << a << std::endl;
    std::cout << *(&a + 1) << std::endl;

    int *ptr = (int *)(&a + 1);

    cout << *(a + 1) << *(ptr - 1);

    return 0;
}