#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace std;

/*
 g++ how_to_redirect.cpp -std=c++11;./a.out
*/

int main() {
  cout << "i am the parent:" << getpid() << endl;
  auto code = fork();
  if (code == 0) { // child
    close(STDOUT_FILENO);
    open("./stdout.out", O_CREAT | O_WRONLY, S_IRWXU);

    // cout与printf不同步，所以不要使用
    // cout << "i am the child:" << getpid();
    printf("i am the child %d\n", getpid());

    char *myargs[3];
    myargs[0] = strdup("echo");
    myargs[1] = strdup("how to implement redirect?");
    myargs[2] = NULL;
    execvp(myargs[0], myargs);

  } else if (code < 0) { // parent
    wait();
  } else {
    cout << "fail to fork in proces:" << getpid() << endl;
    cout << "return value of fork():" << code << endl;
  }
  return 0;
}