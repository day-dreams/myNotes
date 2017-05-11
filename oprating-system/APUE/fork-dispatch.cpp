/*
  fork产生的子进程和父进程之间的调度问题
*/

#include <ctime>
#include <iostream>
#include <unistd.h>

time_t get_current_clock() {
  static time_t t;
  time(&t);
  return t;
}

int main(int argc, char const *argv[]) {
  std::cout << (fork() ? "parent" : "child") << '\n';

  pid_t pid = fork();
  if (pid == 0) {
    std::cout << "子进程内: " << get_current_clock() << '\n';
  } else if (pid > 0) {
    std::cout << "父进程内: " << get_current_clock() << '\n';
  } else {
    std::cout << "error during fork" << '\n';
  }

  return 0;
}
