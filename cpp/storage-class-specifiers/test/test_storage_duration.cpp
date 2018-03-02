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
int main(int argc, char** argv)
{
    Test              object4(4);
    thread_local Test object7(7);
    static Test       object3(3);
    // std::cout << "object5: " << object5.x << std::endl;

    std::thread x(mythread);
    x.join();
    std::thread y(mythread);
    y.join();

    return 0;
}

Test object6(6);
