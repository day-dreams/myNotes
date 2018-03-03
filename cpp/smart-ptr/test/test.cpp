#include <ctime>
#include <iostream>
#include <memory>

using namespace std;

class Test
{
  public:
    Test()
    {
        cout << "object constructed" << endl;
    }
    ~Test()
    {
        cout << "object destructed" << endl;
    }
    int x = 0;

  private:
};

class MyDeleterSingle
{
  public:
    void operator()(Test* ptr)
    {
        std::cout << "custom deleter" << std::endl;
        delete ptr;
    }
};
class MyDeleterMulty
{
  public:
    void operator()(Test* ptr)
    {
        std::cout << "custom deleter" << std::endl;
        delete[] ptr;
    }
};

int main(int argc, char** argv)
{
    MyDeleterMulty  dmul;
    MyDeleterSingle dsgl;

    {
        shared_ptr<Test> sptr(new Test[10](), dmul);
        std::cout << sptr.use_count() << std::endl;
        auto copyp = sptr;
        std::cout << sptr.use_count() << std::endl;
    }

    return 0;
}