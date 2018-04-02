#include "component.h"

extern const int min_xbegin = 0, max_xbegin = 70;
extern const int min_ybegin = 0, max_ybegin = 30;
char vstdout[max_xbegin - min_xbegin][max_ybegin - min_ybegin] = {0};

int Graphic::global_id = 0;

#define clear() printf("\033[H\033[J")

void update_terminal()
{
    clear();
    for (int i = min_xbegin; i != max_ybegin; ++i)
    {
        for (int j = min_ybegin; j != max_ybegin; ++j)
        {
            if (vstdout[i][j] != 0)
            {
                std::cout << "█";
            }
            else
            {
                std::cout << "  ";
            }
        }
        std::cout << std::endl;
    }
}
