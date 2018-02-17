#include "buddy.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

void page_block::show(const struct_page* const begin)
{
    std::cout << "[" << this->begin - begin
              << this->begin - begin + std::pow(2, this->order) << ")" << std::endl;
}
buddySystem::buddySystem(const unsigned int max_order)
{
    unsigned int page_num;

    this->max_order = max_order;

    // 初始化mem_map
    page_num      = pow(2, max_order);
    this->mem_map = ::new struct_page[page_num];

    // 初始化flist
    this->flist.resize(max_order + 1);

    /* 初始化block */
    page_block raw;
    raw.begin = mem_map;
    raw.order = max_order;
    raw.valid = true;
    flist[max_order].push_back(raw);
}

buddySystem::~buddySystem()
{
    ::delete[] this->mem_map;
}

page_block buddySystem::alloc_pages(const unsigned int order)
{
    if (order > max_order)
    {
        throw std::invalid_argument("请求order必须小于max_order");
    }

    for (int i = order; i != max_order + 1; ++i)
    {
        if (flist[i].empty())
            continue;

        page_block block = flist[i].front();
        flist[i].pop_front();

        if (block.order == order)
        {
            return block;
        }
        else  //分割,再次尝试分配
        {
            page_block a, b;

            a.begin = block.begin;
            a.order = block.order - 1;
            a.valid = false;
            b.begin = block.begin + static_cast<int>(pow(2, a.order));
            b.order = block.order - 1;
            b.valid = true;

            flist[a.order].push_back(a);
            flist[b.order].push_back(b);

            // show();
            return alloc_pages(order);
        }
    }
    throw std::runtime_error("没有合适的可用页块");
}

list<page_block>::iterator buddySystem::find_buddy(const page_block& block)
{
    struct_page* buddy_begin;

    int num = (block.begin - mem_map) / static_cast<size_t>(pow(2, block.order));
    // std::cout << "begin" << (block.begin - mem_map) << std::endl;
    // std::cout << "num" << num << std::endl;
    if (num % 2 == 0)
    {
        buddy_begin = block.begin + static_cast<int>(pow(2, block.order));
    }
    else
    {
        buddy_begin = block.begin - static_cast<int>(pow(2, block.order));
    }

    // std::cout << "begin:" << buddy_begin - mem_map << std::endl;

    for (auto x = flist[block.order].begin(); x != flist[block.order].end(); ++x)
    {
        if (x->begin == buddy_begin)
            return x;
    }

    throw std::runtime_error("buddy not found.");
}

void buddySystem::free(page_block& block)
{
    block.valid = false;

    try
    {
        // buddy found.
        auto buddy = find_buddy(block);

        // 融合之
        block.begin = std::min(block.begin, buddy->begin);
        block.order = block.order + 1;

        // 插入工作委托给递归
        this->free(block);

        // 删除buddy
        flist[buddy->order].erase(buddy);
    }
    catch (std::runtime_error& err)
    {
        // no buddy found,

        // throw err;

        //插入到flist
        flist[block.order].push_back(block);
    }
}

void buddySystem::show()
{
    for (int i = 0; i != flist.size(); ++i)
    {
        std::cout << "order " << i << "\t";

        for (auto& x : flist[i])
        {
            std::cout << "[" << x.begin - mem_map << ","
                      << x.begin + static_cast<int>(pow(2, x.order)) - mem_map
                      << ")" << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}