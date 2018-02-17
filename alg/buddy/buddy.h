#ifndef BUDDY_SYSTEM
#define BUDDY_SYSTEM

#include <list>
#include <vector>
using std::list;
using std::vector;

class struct_page
{
  public:
    int users = 0;  //页用户数量
};

class page_block
{
  public:
    int          order;
    bool         valid; /* 是否可使用 */
    struct_page* begin; /* 第一个页 */

    void show(const struct_page* const begin);
};

class buddySystem
{
  public:
    // ctor and dtor
    buddySystem(unsigned int max_order);
    ~buddySystem();

    // 分配页块
    page_block alloc_pages(const unsigned int order);

    // 销毁页块,如果block存在空闲的伙伴,则融合后递归销毁新的页块
    void free(page_block& block);

    // 打印空闲页块情况到stdout
    void show();

    const struct_page* const get_memmap()
    {
        return mem_map;
    }

  private:
    struct_page*             mem_map;
    vector<list<page_block>> flist;
    unsigned int             max_order;

    // 查找buddy
    list<page_block>::iterator find_buddy(const page_block& block);
};

#endif