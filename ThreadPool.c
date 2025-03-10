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

//�����߳�
void* worker(void* arg) {
	ThreadPool* t = (ThreadPool*)arg;
	while (1) {
		pthread_mutex_lock(&t->mutexpool);//ֻ��һ���߳̿���ʹ���̳߳ض��� ����
		//�ж���������Ƿ�Ϊ��
		while (t->queueSize == 0 && t->shutdown == 0) {//�̳߳ػ����ڲ����������Ϊ�գ����������������
			pthread_cond_wait(&t->notEmpty,&t->mutexpool);//������ʱ���Զ�������
			if (t->exitNum > 0) {//���߳���ɱ
				t->exitNum--;
				t->liveNum--;
				pthread_mutex_unlock(&t->mutexpool);
				threadexit(t);
			}
		}
		if (t->shutdown == 1) {//���̳߳عر�ʱ�������򿪣����˳����߳�
			pthread_mutex_unlock(&t->mutexpool);
			threadexit(t);
		}
		//��ʼ��������л�ȡ������
		Task task;
		task.function = t->taskQ[t->queueFront].function;
		task.arg = t->taskQ[t->queueFront].arg;
		t->queueFront = (t->queueFront + 1) % t->queueCapacity;//ͷ�ڵ������һλ
		t->queueSize--;
		pthread_mutex_unlock(&t->mutexpool);//����Ҫ�����̳߳ض��󣬽���
		pthread_cond_signal(&t->notFull);
		pthread_mutex_lock(&t->mutexBusy);
		t->busyNum++;//��æ�߳�+1
		pthread_mutex_unlock(&t->mutexBusy);
		printf("%ldstart working",pthread_self());
		(*task.function)(task.arg);//����
		free(task.arg);
		task.arg = NULL;
		pthread_mutex_lock(&t->mutexBusy);
		t->busyNum--;//��æ�߳�-1
		pthread_mutex_unlock(&t->mutexBusy);
		
	}

	return NULL;

}

//�������߳�
void* manager(void* arg) {//����Ϊ�̳߳ض���
	ThreadPool* t = (ThreadPool*)arg;
	while (!t->shutdown) {
		//ÿ��3s���һ��
		sleep(3);
		//ȡ���̳߳�������������͵�ǰ�̵߳�����
		pthread_mutex_lock(&t->mutexpool);
		int tasknum = t->queueSize;
		int threadnum = t->liveNum;
		pthread_mutex_unlock(&t->mutexpool);
		//ȡ��æ���̵߳ĸ���
		pthread_mutex_lock(&t->mutexBusy);
		int busynum = t->busyNum;
		pthread_mutex_unlock(&t->mutexBusy);
		//����߳�  �����涨�������߳���>����߳���&&����߳���<����߳����� ÿ�������������߳�
		
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
		//�����߳� �����涨����������*2<����߳����� ÿ������2���߳�
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


//�����̳߳ز���ʼ��
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
		pool->liveNum = minnum;    // ����С�������
		pool->exitNum = 0;

		if (pthread_mutex_init(&pool->mutexpool, NULL) != 0 ||
			pthread_mutex_init(&pool->mutexBusy, NULL) != 0 ||
			pthread_cond_init(&pool->notEmpty, NULL) != 0 ||
			pthread_cond_init(&pool->notFull, NULL) != 0)
		{
			printf("mutex or condition init fail...\n");
			break;
		}

		// �������
		pool->taskQ = (Task*)malloc(sizeof(Task) * queueCapacity);
		pool->queueCapacity = queueCapacity;
		pool->queueSize = 0;
		pool->queueFront = 0;
		pool->queueRear = 0;

		pool->shutdown = 0;

		// �����߳�
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

	////��ʼ��������������
	//pthread_mutex_init(&t->mutexpool, NULL);
	//pthread_mutex_init(&t->mutexBusy, NULL);
	//pthread_cond_init(&t->notFull, NULL);
	//pthread_mutex_init(&t->notEmpty, NULL);

	////�������
	//t->taskQ = (Task*)(sizeof(Task) * queueCapacity);
	//t->queueCapacity = queueCapacity;
	//t->queueSize = 0;
	//t->queueFront = 0;
	//t->queueRear = 0;

	//t->shutdown = 0;
	////�����߳�
	//pthread_create(&t->managerId, NULL, manager, t);
	//for (int i = 0; i < minnum; i++) {
	//	pthread_create(&t->threadIds[i], NULL, worker, t);//��������а����˺�����ַ�Ͳ���
	//}
	//return pool;
}

//�����̳߳�
int threadPoolDestroy(ThreadPool* pool) {
	if (pool == NULL) return -1;
	pool->shutdown = 1;
	pthread_join(pool->managerId, NULL);//���չ������߳�
	//�����������̣߳�ʹ���̻߳���
	for (int i = 0; i < pool->liveNum; i++) {
		pthread_cond_signal(&pool->notEmpty);
	}
	//���ն��ڴ�
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

//���̳߳��������
void threadpooladd(ThreadPool* pool, void (*func)(void*), void* arg) {
	pthread_mutex_lock(&pool->mutexpool);
	while (pool->queueSize==pool->queueCapacity&&!pool->shutdown) {//��������������̳߳�
		pthread_cond_wait(&pool->notFull,&pool->mutexpool);
	}
	if (pool->shutdown) {//�Ѿ���ɱ����
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

//��ȡ�������̸߳���
int threadBusy(ThreadPool* pool) {
	pthread_mutex_lock(&pool->mutexBusy);
	int t = pool->busyNum;
	pthread_mutex_unlock(&pool->mutexBusy);
	return t;
}

//��ȡ���ŵ��̸߳���
int threadAlive(ThreadPool* pool) {
	pthread_mutex_lock(&pool->mutexpool);
	int t = pool->liveNum;
	pthread_mutex_unlock(&pool->mutexpool);
	return t;
}
