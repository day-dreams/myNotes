#include <iostream>
#include <thread>

class Test
{
  public:
    Test(int x = 0) : x(x)
    {
        std::cout << "Object " << x << " construct. " << std::endl;
    }
    ~Test()
    {
        std::cout << "Object " << x << " destruct. " << std::endl;
    }

    int x;
};

Test              object2(2);
static Test       object1(1);
thread_local Test object5(5);

void mythread()
{
    std::cout << "subtherad entered" << std::endl;
    std::cout << "object5: " << object5.x << std::endl;
    std::cout << "subtherad exited" << std::endl;
}

void fun()
{
    static Test object19(19);
}
int main(int argc, char** argv)
{
    Test object4(4);

    {
        static Test object222;
        static Test object3(3);
        static Test object111;
    }
    static Test object11(11);

    fun();

    // thread_local Test object7(7);
    // std::cout << "object5: " << object5.x << std::endl;

    new Test(10);

    // std::thread x(mythread);
    // x.join();
    // std::thread y(mythread);
    // y.join();

    return 0;
}

Test object6(6);
