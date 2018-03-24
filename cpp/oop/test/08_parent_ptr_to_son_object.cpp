#include <iostream>
using namespace std;
class base
{
  public:
    int a;
    base()
    {
        cout << "base constructor" << endl;
    }
    ~base()
    {
        cout << "base destructor" << endl;
    }
};
class derive1 : public base
{
  public:
    derive1()
    {
        cout << "derive1 constructor" << endl;
    }
    virtual ~derive1()
    {
        cout << "derive1 destructor" << endl;
    }
};
class derive2 : public derive1
{
  public:
    derive2()
    {
        cout << "derive2 constructor" << endl;
    }
    ~derive2()
    {
        cout << "derive2 destructor" << endl;
    }
};
int main()
{
    cout << sizeof(base) << " " << sizeof(derive1) << " " << sizeof(derive2) << endl;
    derive2* d = new derive2();
    base*    b = d;
    std::cout << d << std::endl;
    std::cout << b << std::endl;

    delete b; /* error! b并不指向new出来的首地址 */
}