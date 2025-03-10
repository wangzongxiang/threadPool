#pragma once
#include <pthread.h>
#include<stdlib.h>


//������еĽṹ��
typedef struct Task {
	void(*function)(void* arg);//����ָ��
	void* arg;
}Task;

//�̳߳ؽṹ��
typedef struct ThreadPool {
	//�������
	Task* taskQ;
	int queueCapacity;//����
	int queueSize;//��ǰ�������
	int queueFront;//��ͷ
	int queueRear;//��β

	pthread_t managerId; //�������߳�
	pthread_t* threadIds;//�����߳�
	int minNum;//��С�߳���
	int maxNum;//����߳���
	int busyNum;//��æ�̵߳ĸ���
	int liveNum;//�����߳���
	int exitNum;//ɱ���߳���
	pthread_mutex_t mutexpool;//�������̳߳�
	pthread_mutex_t mutexBusy;//æ�̸߳�����������
	pthread_cond_t notFull;//�ж���������Ƿ���
	pthread_cond_t notEmpty;//�ж���������Ƿ��

	int shutdown;//�Ƿ������̳߳�
}ThreadPool;

void* worker(void* arg);
void* manager(void* arg);

void threadexit(ThreadPool* arg);//�����߳��˳�

//�����̳߳ز���ʼ��
ThreadPool* threadPoolCreate(int maxnum, int minnum, int queuesize);
 

//�����̳߳�
int threadPoolDestroy(ThreadPool* pool);

//���̳߳��������
void threadpooladd(ThreadPool* pool,void (*func)(void*),void* arg);

//��ȡ�������̸߳���
int threadBusy(ThreadPool* pool);

//��ȡ���ŵ��̸߳���
int threadAlive(ThreadPool* pool);