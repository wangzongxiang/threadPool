# threadPool
#线程池包含三部分：
#1.任务队列：用于存放需要执行的函数地址和函数执行时需要的参数
#2.工作线程：用于执行存放在任务队列中的函数
#3.管理者线程：用于管理线程池，当任务队列为空时，哪怕工作线程抢到了时间片也不能执行也就是会被阻塞.
#多线程需要注意的问题，因为多线程是访问同一个进程的内存空间，因此当线程之间对一个地址上的数据进行读写时，可能会出现问题，这时就需要使用读写锁，条件变量，互斥锁等来实现线程同步，实现的原理就是在在某一时刻只允许拿到锁的线程可以对内存空间进行读写操作
