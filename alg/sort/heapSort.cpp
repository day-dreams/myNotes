
#include <iostream>

template <class T> void print(T* begin, T* end)
{
    while (begin != end)
    {
        std::cout << *begin << " ";
        ++begin;
    }
    std::cout << std::endl;
}

/* 最小堆 */
template <class Ite> void make_heap(Ite begin, Ite back)
{
    Ite ite = back;
    while (ite >= begin)
    {
        Ite left, right;
        Ite cur = ite;

        // make a sub heap
        while (true)
        {
            left = begin + (cur - begin) * 2 + 1;

            // std::cout << "1 left:" << left - begin << std::endl;
            if (left > back)
            {
                break;
            }
            // std::cout << "2 left:" << left - begin << std::endl;

            right = left + 1;
            if (right > back)
            {
                if (*cur > *left)
                {
                    std::swap(*cur, *left);
                }
                break;
            }

            bool cur_g_left   = *cur > *left;
            bool cur_g_right  = *cur > *right;
            bool left_g_right = *left > *right;

            if (cur_g_left)
            {
                if (left_g_right)
                {
                    // cur,left>right
                    std::swap(*cur, *right);
                    cur = right;
                }
                else
                {
                    // cur,right>left
                    std::swap(*cur, *left);
                    cur = left;
                }
            }
            else
            {
                if (cur_g_right)
                {
                    // left,cur>right
                    std::swap(*cur, *right);
                    cur = right;
                }
                else
                {
                    // left, right > cur
                    break;
                }
            }
        }

        --ite;
    }
}

/* 取出堆顶元素,放到最后的位置,并维护堆的性质 */
template <class Ite> void pop_heap(Ite begin, Ite back)
{
    std::swap(*begin, *back);
    back = back - 1;

    Ite left, right;
    Ite cur = begin;

    // make a sub heap
    while (true)
    {
        left = begin + (cur - begin) * 2 + 1;

        if (left > back)
        {
            break;
        }

        right = left + 1;
        if (right > back)
        {
            if (*cur > *left)
            {
                std::swap(*cur, *left);
            }
            break;
        }

        bool cur_g_left   = *cur > *left;
        bool cur_g_right  = *cur > *right;
        bool left_g_right = *left > *right;

        if (cur_g_left)
        {
            if (left_g_right)
            {
                // cur,left>right
                std::swap(*cur, *right);
                cur = right;
            }
            else
            {
                // cur,right>left
                std::swap(*cur, *left);
                cur = left;
            }
        }
        else
        {
            if (cur_g_right)
            {
                // left,cur>right
                std::swap(*cur, *right);
                cur = right;
            }
            else
            {
                // left, right > cur
                break;
            }
        }
    }
}

int main(int argc, char** argv)
{
    int    nums[] = {-4, -5, -2, 5, 0, 9, 1, 19};
    size_t size   = sizeof(nums) / sizeof(int);

    print(nums, nums + size);

    make_heap(nums, nums + size - 1);

    for (int i = 0; i != size; ++i)
    {
        pop_heap(nums, nums + size - 1 - i);

        std::cout << *(nums + size - 1 - i) << ' ';
    }

    std::cin.get();
    return 0;
}