#include "composites.h"

int main(int argc, char** argv)
{
    update_terminal();

    Line line(1, 1, 10, 10);
    line.Drow();

    update_terminal();

    return 0;
}