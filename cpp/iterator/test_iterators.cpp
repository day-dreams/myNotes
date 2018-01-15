#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>

/*
    OutputIterator and InputIterator tests
 */
void test_io_iterator() {
  std::istringstream str("0.1 0.2 0.3 0.4");
  std::partial_sum(std::istream_iterator<double>(str),
                   std::istream_iterator<double>(),
                   std::ostream_iterator<double>(std::cout, " "));
}

int main(int argc, char **argv) {
  test_io_iterator();

  return 0;
}