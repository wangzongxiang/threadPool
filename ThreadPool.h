#pragma once
#include <pthread.h>
#include<stdlib.h>


//任务队列的结构体
typedef struct Task {
	void(*function)(void* arg);//函数指针
	void* arg;
}Task;

//线程池结构体
typedef struct ThreadPool {
	//任务队列
	Task* taskQ;
	int queueCapacity;//容量
	int queueSize;//当前任务个数
	int queueFront;//队头
	int queueRear;//队尾

	pthread_t managerId; //管理者线程
	pthread_t* threadIds;//工作线程
	int minNum;//最小线程数
	int maxNum;//最大线程数
	int busyNum;//繁忙线程的个数
	int liveNum;//存活的线程数
	int exitNum;//杀死线程数
	pthread_mutex_t mutexpool;//锁整个线程池
	pthread_mutex_t mutexBusy;//忙线程个数变量的锁
	pthread_cond_t notFull;//判断任务队列是否满
	pthread_cond_t notEmpty;//判断任务队列是否空

	int shutdown;//是否销毁线程池
}ThreadPool;

void* worker(void* arg);
void* manager(void* arg);

void threadexit(ThreadPool* arg);//工作线程退出

//创作线程池并初始化
ThreadPool* threadPoolCreate(int maxnum, int minnum, int queuesize);
 

//销毁线程池
int threadPoolDestroy(ThreadPool* pool);

//给线程池添加任务
void threadpooladd(ThreadPool* pool,void (*func)(void*),void* arg);

//获取工作的线程个数
int threadBusy(ThreadPool* pool);

//获取活着的线程个数
int threadAlive(ThreadPool* pool);