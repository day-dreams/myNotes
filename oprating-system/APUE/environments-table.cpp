/*
  unix下,程序环境表输出
*/

#include <iostream>
#include <stdio.h>

/*猜测由exec()执行时提供*/
extern char **environ;

int main(int argc, char const *argv[]) {
  // for (size_t index = 0; environ[index] != nullptr; ++index) {
  //   std::cout << environ[index] << '\n';
  // }

  std::cout << getenv("SHELL") << '\n';
  std::cout << getenv("USER") << '\n';
  std::cout << getenv("PWD") << '\n';

  /*对于不存在环境的查询,会导致破坏cout的状态*/
  printf("--%p\n", getenv("USE"));
  std::cout << getenv("USR") << '\n';

  /*cout已经失效,只有printf才会打印出来*/
  printf("- %d\n", std::cout.good());
  printf("- end..\n");
  std::cout << std::cout.good() << '\n';
  std::cout << "end.." << '\n';
  return 0;
}
