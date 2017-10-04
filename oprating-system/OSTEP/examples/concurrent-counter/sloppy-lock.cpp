#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

class ConcurrentCounter {
private:
  mutex global_lock;
  int global_count = 0;
  int treshold = 5;
  int local_count = 0;
  int operate_times = 0;

public:
  void add() {
    ++local_count;
    ++operate_times;
    if (operate_times == ConcurrentCounter::treshold) {
      global_lock.lock();
      global_count += local_count;
      global_lock.unlock();
      local_count = 0;
      operate_times = 0;
    }
  }

  int get() {
    global_lock.lock();
    auto x = global_count;
    global_lock.unlock();
    return x;
  }
};

ConcurrentCounter counter;

void mythread() {
  for (int i = 0; i != 10000000; ++i)
    counter.add();
}

int main(int argc, char **argv) {
  auto count = atoi(argv[1]);

  function<void()> fun(mythread);
  vector<thread> threads;

  auto start = chrono::system_clock::now();

  for (int i = 0; i < count; ++i)
    threads.push_back(thread(fun));
  for (int i = 0; i < count; ++i)
    threads[i].join();

  auto end = chrono::system_clock::now();

  std::chrono::duration<double> timespent = end - start;
  cout << "time spent:" << timespent.count() << endl;

  return 0;
}
