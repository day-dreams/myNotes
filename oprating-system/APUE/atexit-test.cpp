#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdlib.h>

void on_process_exit() {
  std::time_t tt = std::time(nullptr);
  std::tm t = *std::localtime(&tt);
  std::get_time(&t, "%Y-%b-%d %H:%M:%S");
  std::cout << "process exited at: " << std::put_time(&t, "%Y-%b-%d %H:%M:%S")
            << '\n';
}

int main(int argc, char const *argv[]) {
  atexit(on_process_exit);
  exit(0);
  return 0;
}
