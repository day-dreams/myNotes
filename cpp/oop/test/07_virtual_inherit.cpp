#include <iostream>

class B1
{
  public:
    int x = 1;

  private:
};

class B2
{
  public:
    int y = 2;

  private:
};

class Drive : virtual public B1  //, virtual public B2
{
  public:
    int z = 3;
};

int main(int argc, char** argv)
{
    Drive d;

    int* ptr = (int*)(&d);

    // vbptr 8
    {
        int* vb = *(int**)(&d);
        std::cout << vb + 0 << ' ' << *(vb + 0) << std::endl;
        std::cout << vb + 1 << ' ' << *(vb + 1) << std::endl;
    }

    std::cout << ptr + 0 << ' ' << *(ptr + 0) << std::endl;
    std::cout << ptr + 1 << ' ' << *(ptr + 1) << std::endl;

    // z 4
    std::cout << ptr + 2 << ' ' << *(ptr + 2) << std::endl;

    // x 4
    std::cout << ptr + 3 << ' ' << *(ptr + 3) << std::endl;

    // ??
    // std::cout << *(ptr + 4) << std::endl;
    // std::cout << *(ptr + 5) << std::endl;

    std::cout << "sieof d: " << sizeof(d) << std::endl;
    std::cin.get();
    return 0;
}