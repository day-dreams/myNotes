#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

mutex m;
condition_variable cv;
int data;

void wait_thread(int num) {
  unique_lock<mutex> lock(m);
  cout << "ChildThread " << num << ",lock acquired." << endl;

  //   while (data != 12345)
  //     cv.wait(lock);

  cv.wait(lock, []() { return data == 12345; });

  cout << "ChildThread " << num << ",data recved:" << data << endl;
}

void signal_thread(int time) {
  //   this_thread::sleep_for(chrono::seconds(time));
  unique_lock<mutex> lock(m);
  data = 12345;
  cout << "SignalThread, notifying" << endl;
  cv.notify_all();
}

int main() {
  vector<thread> threads;
  thread signal;
  for (int i = 0; i != 5; ++i) {
    if (i == 3)
      signal = thread(signal_thread, 3);
    threads.push_back(thread(wait_thread, i));
  }
  for (auto &x : threads)
    x.join();
  signal.join();

  return 0;
}