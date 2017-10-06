#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <thread>
#include <vector>
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
condition_variable fullcondvar, emptycondvar;
mutex fullmutex, emptymutex, buffermutex;

random_device engine;
int generateRandomInt() { return engine(); }

void produce() {
  while (true) {
    this_thread::sleep_for(chrono::seconds(3));
    {
      unique_lock<mutex> lock(emptymutex);
      emptycondvar.wait(lock, []() {
        buffermutex.lock();
        auto size = buffer.getSize();
        buffermutex.unlock();
        return size == 0;
      });
    }
    buffermutex.lock();
    cout << "produceThread: adding product.." << endl;
    buffer.put(generateRandomInt());
    buffermutex.unlock();

    fullcondvar.notify_one(); //通知一个消费者开始消费
  }
}

void consume() {
  while (true) {
    {
      unique_lock<mutex> lock(fullmutex);
      fullcondvar.wait(lock, []() {
        buffermutex.lock();
        auto size = buffer.getSize();
        buffermutex.unlock();
        return size > 0;
      });
    }

    buffermutex.lock();
    auto x = buffer.get();
    buffermutex.unlock();
    cout << "consumeThread: got an element " << x << endl;

    emptycondvar.notify_all(); //通知所有生产者开始生产
  }
}

int main() {
  vector<thread> producer, consumer;
  for (int i = 0; i != 5; ++i) {
    producer.push_back(thread(produce));
  }
  for (int i = 0; i != 10; ++i) {
    consumer.push_back(thread(consume));
  }

  for (auto &t : producer)
    t.join();
  for (auto &t : consumer)
    t.join();

  return 0;
}