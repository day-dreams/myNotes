#include "myallocator.h"
#include <vector>

int main(int argc, char **argv) {
    std::vector<int, moon::allocator<int>> col;
    col.push_back(1);
    return 0;
}