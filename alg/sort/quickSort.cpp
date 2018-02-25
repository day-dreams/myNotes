#include <algorithm>
#include <iostream>
#include <iterator>

template <class T> void print(T* begin, T* end)
{
    while (begin != end)
    {
        std::cout << *begin << " ";
        ++begin;
    }
    std::cout << std::endl;
}

template <class T> T* partition(T* begin, T* end, const T& pivot)
{
    T* back = end - 1;

    while (begin < back)
    {
        while (*begin < pivot && begin != back)
            ++begin;

        if (begin == back)
            return begin;

        //此处可以优化
        std::swap(*begin, *back);

        while (*back >= pivot && begin != back)
            --back;

        // std::cout << "begin:" << begin - num << ',' << *begin << ", ";
        // print(num, num + sizeof(num) / sizeof(int));
    }
    return begin;
}

template <class T> void quick_sort(T* begin, T* end)
{
    // std::cout << "partition size:" << end - begin << std::endl;
    // std::cin.get();
    if (begin >= end)
        return;

    auto mid = partition(begin, end, *begin);

    quick_sort(begin, mid - 1);
    quick_sort(mid + 1, end);
}

int main(int argc, char** argv)
{
    int num[] = {884, 430, 229, 384, 497, 295, 703, 391, 738, 726, 143, 750,
                 918, 666, 889, 59,  178, 687, 354, 523, 155, 929, 300, 159,
                 80,  256, 36,  339, 832, 685, 979, 218, 55,  403, 774, 604,
                 794, 134, 6,   138, 403, 667, 679, 521, 15,  784, 51,  744,
                 3,   783, 56,  812, 46,  720, 619, 544, 577, 650, 543, 41,
                 16,  962, 455, 132, 139, 791, 675, 699, 508, 631, 645, 816,
                 221, 450, 350, 256, 81,  461, 282, 851, 714, 78,  31,  192,
                 941, 308, 903, 204, 733, 34,  66,  361, 556, 807, 271, 118,
                 320, 465, 589, 771};

    size_t size = sizeof(num) / sizeof(int);

    std::cout << "before qs: ";
    print(num, num + size);

    // auto ptr = partition(num, num + size, 3);
    // std::cout << ptr - num << std::endl;
    quick_sort(num, num + size);

    std::cout << "after qs: ";
    print(num, num + size);

    return 0;
}