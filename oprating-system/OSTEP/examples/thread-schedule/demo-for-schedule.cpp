#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *mythread(void *arg) {
  auto x = (*(int *)arg);
  for (int i = 0; i < 5; ++i) {
    sleep(1);
    printf("thread %d\n", x);
  }
}

int main(int argc, char **argv) {
  pthread_t threads[5];

  int y[5];
  for (int i = 0; i != 5; ++i) {
    y[i] = i;
    pthread_create(&threads[i], nullptr, mythread, (void *)&y[i]);
  }
  for (int i = 0; i != 5; ++i) {
    pthread_join(threads[i], nullptr);
  }
  return 0;
}