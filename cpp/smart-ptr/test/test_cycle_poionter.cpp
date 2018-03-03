#include <iostream>
#include <memory>

using namespace std;

class Test1;
class Test2;

class Test1
{
  public:
    // weak_ptr<Test2> wptr;
    shared_ptr<Test2> wptr;  // error,memory leak

    Test1()
    {
        std::cout << "object1 constructed" << std::endl;
    }
    ~Test1()
    {
        std::cout << "object1 destructed" << std::endl;
    }
};

class Test2
{
  public:
    // weak_ptr<Test1> wptr;
    shared_ptr<Test1> wptr;  // error,memory leak

    Test2()
    {
        std::cout << "object2 constructed" << std::endl;
    }
    ~Test2()
    {
        std::cout << "object2 destructed" << std::endl;
    }
};

int main(int argc, char** argv)
{
    shared_ptr<Test1> test1(new Test1);

    shared_ptr<Test2> test2(new Test2);

    test1->wptr = test2;

    test2->wptr = test1;

    return 0;
}