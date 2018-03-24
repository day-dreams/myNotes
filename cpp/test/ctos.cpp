#include <iostream>

class Object
{
  public:
    Object()
    {
        std::cout << "default ctor" << std::endl;
    }
    Object(Object&& other)
    {
        std::cout << "move ctor" << std::endl;
    }
    Object operator=(Object&& other)
    {
        std::cout << "move assignment" << std::endl;
    }

    Object(const Object& other)
    {
        std::cout << "copy ctor" << std::endl;
    }
    Object operator=(const Object& other)
    {
        std::cout << "copy assignment" << std::endl;
    }
    ~Object()
    {
        // std::cout << "dtor" << std::endl;
    }

  private:
};
int main(int argc, char** argv)
{
    Object x;            /* default cotr */
    Object y = Object(); /* default ctor */
    Object z = y;        /* copy ctor */

    Object u(Object());            /* copy ctor */
    Object v(std::move(Object())); /* move ctor */

    Object w(x);            /* copy ctor */
    Object a(std::move(x)); /* mov ctor */

    z = y;            /* copy = */
    z = std::move(y); /* move = */
    return 0;
}