#include <iostream>
#include <mutex>
#include <queue>
#include <semaphore.h>
#include <thread>
#include <vector>

using namespace std;

class ReadWriteLock {
private:
  sem_t writesem;
  int readers;
  mutex lock;

public:
  ReadWriteLock() {
    readers = 0;
    sem_init(&writesem, 0, 1);
  }

  void getWriteLock() { sem_wait(&writesem); }
  void realeaseWriteLock() { sem_post(&writesem); }

  void getReadLock() {
    if (readers == 0)
      sem_wait(&writesem);
    lock.lock();
    ++readers;
    lock.unlock();
  }
  void realeaseReadLock() {
    lock.lock();
    --readers;
    lock.unlock();
    if (readers == 0)
      sem_post(&writesem);
  }
};

ReadWriteLock rwlock;

void read() {
  while (true) {
    rwlock.getReadLock();
    cout << "ReadThread: i am reading.." << endl;
    rwlock.realeaseReadLock();
  }
}

void write() {
  while (true) {
    // this_thread::sleep_for(chrono::seconds(3));
    rwlock.getWriteLock();
    cout << "WriteThread: i am writing.." << endl;
    rwlock.realeaseWriteLock();
  }
}

int main() {
  vector<thread> readers, writers;

  for (int i = 0; i != 3; ++i)
    writers.push_back(thread(write));
  for (int i = 0; i != 5; ++i)
    readers.push_back(thread(read));

  for (auto &t : writers)
    t.join();
  for (auto &t : readers)
    t.join();

  return 0;
}