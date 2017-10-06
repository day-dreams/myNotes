#include <chrono>
#include <iostream>
#include <queue>
#include <semaphore.h>
#include <thread>

using namespace std;

template <typename T> class buffer_t {
private:
  queue<T> buffer;

public:
  int getSize() { return buffer.size(); }
  void put(const T &ele) { buffer.push(move(ele)); }
  T get() {
    auto rv = buffer.front();
    buffer.pop();
    return rv;
  }
};

buffer_t<int> buffer;
sem_t full, empty, mutex;

void produce() {
  while (true) {
    this_thread::sleep_for(chrono::seconds(1));
    sem_wait(&empty);
    sem_wait(&mutex);
    buffer.put(1);
    sem_post(&mutex);
    sem_post(&full);
  }
}

void consume() {
  while (true) {
    sem_wait(&full);

    sem_wait(&mutex);
    auto x = buffer.get();
    sem_post(&mutex);

    sem_post(&empty);
    cout << "consumeThread: got an ele " << x << endl;
  }
}

int main() {

  sem_init(&full, 0, 0);
  sem_init(&empty, 0, 10); //缓冲区最大容量为10
  sem_init(&mutex, 0, 1);

  vector<thread> producers, consumers;

  for (int i = 0; i != 20; ++i)
    consumers.push_back(thread(consume));
  for (int i = 0; i != 15; ++i)
    producers.push_back(thread(produce));

  for (auto &t : producers)
    t.join();
  for (auto &t : consumers)
    t.join();

  return 0;
}