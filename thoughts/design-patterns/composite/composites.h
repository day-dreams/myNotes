#ifndef COMPOSITES_H
#define COMPOSITES_H

#include "component.h"

extern const int min_xbegin = 0, max_xbegin = 70;
extern const int min_ybegin = 0, max_ybegin = 30;

/* 虚拟终端 */
extern char vstdout[max_xbegin - min_xbegin][max_ybegin - min_ybegin];

class Line : public Graphic
{
  public:
    /* TODO:添加范围检查 */
    Line(int x_begin, int y_begin, int x_end, int y_end)
    {
        this->x_begin = x_begin;
        this->x_end   = x_end;
        this->y_begin = y_begin;
        this->y_end   = y_end;
    }

    virtual void Drow()
    {
        int x = x_begin, y = y_begin;
        for (; x != x_end; ++x, ++y)
        {
            vstdout[x][y] = 1;
        }
    }

  private:
    int x_begin, x_end, y_begin, y_end;
};

class Picture : public Graphic
{
  public:
  private:
};

#endif