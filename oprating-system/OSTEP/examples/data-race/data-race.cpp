#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int x = 0;

void *mythread(void *arg) {
  for (int i = 0; i != 5000; ++i)
    x = x + 1;
}

int main() {

  pthread_t threads[5];

  printf("x=%d\n", x);

  for (int i = 0; i != 5; ++i) {
    pthread_create(&threads[i], nullptr, mythread, nullptr);
  }
  for (int i = 0; i != 5; ++i) {
    pthread_join(threads[i], nullptr);
  }

  printf("x=%d\n", x);

  return 0;
}