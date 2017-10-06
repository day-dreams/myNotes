#include <chrono>
#include <iostream>
#include <semaphore.h>
#include <thread>
#include <vector>

using namespace std;

sem_t s;

void produce_thread() {
  while (true) {
    this_thread::sleep_for(chrono::seconds(1));
    sem_post(&s);
  }
}

void consume_thread() {
  cout << "consumeThread: waiting for a product...." << endl;
  sem_wait(&s);
  cout << "consumeThread: got a product." << endl;
}

int main() {

  sem_init(&s, 0, 0);

  vector<thread> producers, consumers;

  for (int i = 0; i != 7; ++i)
    consumers.push_back(thread(consume_thread));
  for (int i = 0; i != 3; ++i)
    producers.push_back(thread(produce_thread));

  for (auto &t : producers)
    t.join();
  for (auto &t : consumers)
    t.join();

  return 0;
}