#include"ThreadPool.h""


const int number = 2;

void threadexit(ThreadPool* p) {
	pthread_t tid = pthread_self();
	for (int i = 0; i < p->maxNum; i++) {
		pthread_mutex_lock(&p->mutexpool);
		if (p->threadIds[i] == tid) {
			p->threadIds[i] = 0;
		}
		pthread_mutex_unlock(&p->mutexpool);
	}
	pthread_exit(NULL);

}

//工作线程
void* worker(void* arg) {
	ThreadPool* t = (ThreadPool*)arg;
	while (1) {
		pthread_mutex_lock(&t->mutexpool);//只有一个线程可以使用线程池对象 加锁
		//判断任务队列是否为空
		while (t->queueSize == 0 && t->shutdown == 0) {//线程池还存在并且任务队列为空，即无任务可以消费
			pthread_cond_wait(&t->notEmpty,&t->mutexpool);//当阻塞时会自动将锁打开
			if (t->exitNum > 0) {//让线程自杀
				t->exitNum--;
				t->liveNum--;
				pthread_mutex_unlock(&t->mutexpool);
				threadexit(t);
			}
		}
		if (t->shutdown == 1) {//当线程池关闭时，将锁打开，并退出该线程
			pthread_mutex_unlock(&t->mutexpool);
			threadexit(t);
		}
		//开始从任务队列获取消费者
		Task task;
		task.function = t->taskQ[t->queueFront].function;
		task.arg = t->taskQ[t->queueFront].arg;
		t->queueFront = (t->queueFront + 1) % t->queueCapacity;//头节点向后移一位
		t->queueSize--;
		pthread_mutex_unlock(&t->mutexpool);//不需要操作线程池对象，解锁
		pthread_cond_signal(&t->notFull);
		pthread_mutex_lock(&t->mutexBusy);
		t->busyNum++;//繁忙线程+1
		pthread_mutex_unlock(&t->mutexBusy);
		printf("%ldstart working",pthread_self());
		(*task.function)(task.arg);//工作
		free(task.arg);
		task.arg = NULL;
		pthread_mutex_lock(&t->mutexBusy);
		t->busyNum--;//繁忙线程-1
		pthread_mutex_unlock(&t->mutexBusy);
		
	}

	return NULL;

}

//管理者线程
void* manager(void* arg) {//参数为线程池对象
	ThreadPool* t = (ThreadPool*)arg;
	while (!t->shutdown) {
		//每隔3s检测一次
		sleep(3);
		//取出线程池中任务的数量和当前线程的数量
		pthread_mutex_lock(&t->mutexpool);
		int tasknum = t->queueSize;
		int threadnum = t->liveNum;
		pthread_mutex_unlock(&t->mutexpool);
		//取出忙的线程的个数
		pthread_mutex_lock(&t->mutexBusy);
		int busynum = t->busyNum;
		pthread_mutex_unlock(&t->mutexBusy);
		//添加线程  建立规定（任务线程数>存活线程数&&存活线程数<最大线程数） 每次最多添加两个线程
		
		if (tasknum > threadnum && threadnum < t->maxNum) {
			pthread_mutex_lock(&t->mutexpool);
			int cont = 0;
			for (int i = 0; i < t->maxNum &&cont<number&& t->liveNum< t->maxNum; i++) {
				if (t->threadIds[i] == 0) {
					pthread_create(&t->threadIds[i], NULL,worker ,t);
					t->liveNum++;
					cont++;
				}
			}
			pthread_mutex_unlock(&t->mutexpool);
		}
		//销毁线程 建立规定（任务数量*2<存活线程数） 每次销毁2个线程
		if (tasknum * 2 < threadnum&&t->liveNum>t->minNum) {
			pthread_mutex_lock(&t->mutexpool);
			t->exitNum= number;
			pthread_mutex_unlock(&t->mutexpool);
			for (int i = 0; i < number; i++) {
				pthread_cond_signal(&t->notEmpty);
			}
		}
	}
	return NULL;
}


//创作线程池并初始化
ThreadPool* threadPoolCreate(int maxnum, int minnum, int queueCapacity) {
	ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
	do
	{
		if (pool == NULL)
		{
			printf("malloc threadpool fail...\n");
			break;
		}

		pool->threadIds = (pthread_t*)malloc(sizeof(pthread_t) * maxnum);
		if (pool->threadIds == NULL)
		{
			printf("malloc threadIDs fail...\n");
			break;
		}
		memset(pool->threadIds, 0, sizeof(pthread_t) * maxnum);
		pool->minNum = minnum;
		pool->maxNum = maxnum;
		pool->busyNum = 0;
		pool->liveNum = minnum;    // 和最小个数相等
		pool->exitNum = 0;

		if (pthread_mutex_init(&pool->mutexpool, NULL) != 0 ||
			pthread_mutex_init(&pool->mutexBusy, NULL) != 0 ||
			pthread_cond_init(&pool->notEmpty, NULL) != 0 ||
			pthread_cond_init(&pool->notFull, NULL) != 0)
		{
			printf("mutex or condition init fail...\n");
			break;
		}

		// 任务队列
		pool->taskQ = (Task*)malloc(sizeof(Task) * queueCapacity);
		pool->queueCapacity = queueCapacity;
		pool->queueSize = 0;
		pool->queueFront = 0;
		pool->queueRear = 0;

		pool->shutdown = 0;

		// 创建线程
		pthread_create(&pool->managerId, NULL, manager, pool);
		for (int i = 0; i < minnum; ++i)
		{
			pthread_create(&pool->threadIds[i], NULL, worker, pool);
		}
		return pool;
	} while (0);
	return pool;

	//t->threadIds = (pthread_t*)malloc(sizeof(pthread_t) * maxnum);
	//memset(t->threadIds, 0, sizeof(pthread_t) * maxnum);
	//t->liveNum = minnum;
	//t->exitNum = 0;
	//t->busyNum=0;
	//t->maxNum = maxnum;
	//t->minNum = minnum;

	////初始化锁和条件变量
	//pthread_mutex_init(&t->mutexpool, NULL);
	//pthread_mutex_init(&t->mutexBusy, NULL);
	//pthread_cond_init(&t->notFull, NULL);
	//pthread_mutex_init(&t->notEmpty, NULL);

	////任务队列
	//t->taskQ = (Task*)(sizeof(Task) * queueCapacity);
	//t->queueCapacity = queueCapacity;
	//t->queueSize = 0;
	//t->queueFront = 0;
	//t->queueRear = 0;

	//t->shutdown = 0;
	////创建线程
	//pthread_create(&t->managerId, NULL, manager, t);
	//for (int i = 0; i < minnum; i++) {
	//	pthread_create(&t->threadIds[i], NULL, worker, t);//任务队列中包含了函数地址和参数
	//}
	//return pool;
}

//销毁线程池
int threadPoolDestroy(ThreadPool* pool) {
	if (pool == NULL) return -1;
	pool->shutdown = 1;
	pthread_join(pool->managerId, NULL);//回收管理者线程
	//唤醒消费者线程，使得线程回收
	for (int i = 0; i < pool->liveNum; i++) {
		pthread_cond_signal(&pool->notEmpty);
	}
	//回收堆内存
	if (pool->taskQ) {
		free(pool->taskQ);
	}
	if (pool->threadIds) {
		free(pool->threadIds);
	}
	pthread_mutex_destroy(&pool->mutexBusy);
	pthread_mutex_destroy(&pool->mutexpool);
	pthread_cond_destroy(&pool->notEmpty);
	pthread_cond_destroy(&pool->notFull);
	free(pool);
	pool = NULL;
	printf("delete all\n");
	return 0;
}

//给线程池添加任务
void threadpooladd(ThreadPool* pool, void (*func)(void*), void* arg) {
	pthread_mutex_lock(&pool->mutexpool);
	while (pool->queueSize==pool->queueCapacity&&!pool->shutdown) {//任务队列已满且线程池
		pthread_cond_wait(&pool->notFull,&pool->mutexpool);
	}
	if (pool->shutdown) {//已经被杀死了
		pthread_mutex_unlock(&pool->mutexpool);
		return;
	}
	printf("add new thread\n");
	pool->taskQ[pool->queueRear].function =func;
	pool->taskQ[pool->queueRear].arg =arg;
	pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
	pool->queueSize++;
	pthread_mutex_unlock(&pool->mutexpool);
	pthread_cond_signal(&pool->notEmpty);
}

//获取工作的线程个数
int threadBusy(ThreadPool* pool) {
	pthread_mutex_lock(&pool->mutexBusy);
	int t = pool->busyNum;
	pthread_mutex_unlock(&pool->mutexBusy);
	return t;
}

//获取活着的线程个数
int threadAlive(ThreadPool* pool) {
	pthread_mutex_lock(&pool->mutexpool);
	int t = pool->liveNum;
	pthread_mutex_unlock(&pool->mutexpool);
	return t;
}
