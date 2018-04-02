#ifndef COMPONENT_H
#define COMPONENT_H

#include <iostream>

extern const int min_xbegin = 0, max_xbegin = 70;
extern const int min_ybegin = 0, max_ybegin = 30;

/* 虚拟终端 */
extern char vstdout[max_xbegin - min_xbegin][max_ybegin - min_ybegin];

/* 把虚拟终端更新到终端 */
extern void update_terminal();

class Graphic
{
  public:
    Graphic()
    {
        id = ++global_id;
    }

    /* 把自己打印到虚拟终端 */
    virtual void Drow() {}

    virtual void addGraphic(const Graphic& other) {}
    virtual void rmGraphic(const Graphic& other) {}

    virtual bool operator==(const Graphic& other)
    {
        return id == other.id;
    }

  private:
    int id; /* graphic的id */
  public:
    static int global_id;
};

#endif