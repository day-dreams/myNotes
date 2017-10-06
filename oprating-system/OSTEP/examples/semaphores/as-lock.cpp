#include <chrono>
#include <iostream>
#include <semaphore.h>
#include <thread>
using namespace std;

sem_t s;

void wait_thread() {
  cout << "waitThread: waiting..." << endl;
  sem_wait(&s);
  cout << "waitThread: wait finished." << endl;
}

void post_thread() {
  this_thread::sleep_for(chrono::seconds(1));
  cout << "postThread: posting..." << endl;
  sem_post(&s);
  cout << "postThread: post finished." << endl;
}

int main() {

  sem_init(&s, 0, 1);

  thread waiter1(wait_thread), waiter2(wait_thread), poster(post_thread);
  waiter1.join();
  waiter2.join();
  poster.join();

  return 0;
}