#include <ctime>
#include <iostream>
#include <memory>

using namespace std;

int max_count = 5000000;

void mkshared_vs_rawshared()
{
    // make_shared,shared<T>()哪个更快

    {
        auto begin = time(NULL);
        for (int i = 0; i != max_count; ++i)
        {
            make_shared<int>();  //一次分配heap
        }
        auto end = time(NULL);
        std::cout << "use seconds: " << difftime(end, begin) << std::endl;
    }

    {
        auto begin = time(NULL);
        for (int i = 0; i != max_count; ++i)
        {
            shared_ptr<int>(new int);  //两分配heap
        }
        auto end = time(NULL);
        std::cout << "use seconds: " << difftime(end, begin) << std::endl;
    }
}

int main(int argc, char** argv)
{
    mkshared_vs_rawshared();

    return 0;
}