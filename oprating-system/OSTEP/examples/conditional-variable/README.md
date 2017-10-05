C++给条件变量提供了两种wait选择.

* 一种是当被notify的时候就wake up.

* 另一种wait对应两种情况:一种是,在线程调用wait之前,条件已经成立,所以不用wait,直接继续执行;另一种是,线程调用wait前,提交还没有成立,所以进入wait.只有在后续notify了,而且特定条件发生了,才wake up.

第二种明显要好一些,它对条件进行了预防检测.因为很多时候,线程执行顺序不可知:某个负责signal的线程最先执行,通过notify()唤醒条件变量对应队列里的线程,但此时队列里没有wait的线程;其他线程后来才开始执行,调用wait进入条件变量的等待队列后,却因为后执行而错过了notify,与是陷入了无线等待.为了避免这种情况发生,最好在wait之前先检测一下条件是不是成立,如果成立了就不用wait,否则才进入wait.

要**避免错过notify**,有两种方法.

* 使用第一种wait
    ```cpp
    void wait( std::unique_lock<std::mutex>& lock );

    while(condition==false)
        wait(lock);

    ```

* 使用第二种wait
    ```cpp
    template< class Predicate >
    void wait( std::unique_lock<std::mutex>& lock, Predicate pred );

    wait(lock,[](){return condition==false;})
    ```