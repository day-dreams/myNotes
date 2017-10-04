#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

class ConcurrentCounter {
private:
  int count = 0;
  mutex lock;

public:
  void add() {
    lock.lock();
    ++count;
    lock.unlock();
  }
  int get() {
    lock.lock();
    auto x = count;
    lock.unlock();
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
