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
    // cout << "returning:: " << rv << endl;
    return rv;
  }
};

buffer_t<int> buffer;
condition_variable fullcondvar, emptycondvar;
mutex m;

random_device engine;
int generateRandomInt() { return engine(); }

void produce() {
  while (true) {
    this_thread::sleep_for(chrono::seconds(1));
    unique_lock<mutex> lock(m);
    emptycondvar.wait(lock, []() { return buffer.getSize(); });

    cout << "produceThread: adding product.." << endl;
    buffer.put(generateRandomInt());
    fullcondvar.notify_one(); //通知一个消费者开始消费
  }
}

void consume() {
  while (true) {
    unique_lock<mutex> lock(m);
    fullcondvar.wait(lock, []() { return buffer.getSize() > 0; });

    auto x = buffer.get();
    cout << "consumeThread: got an element " << x << endl;

    emptycondvar.notify_one(); //通知一个生产者开始生产
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