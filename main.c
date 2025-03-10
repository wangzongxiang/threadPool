#include<stdio.h>
#include<stdlib.h>
#include"ThreadPool.h"
#include<pthread.h>
#include<string.h>
#include<unistd.h>

void TaskFuntion(void* arg) {
    int t = *(int*)arg;
    printf("threadID:%ld,   number:%d\n", pthread_self(), t);
    sleep(rand() % 3);
}

int main()
{
    ThreadPool* t = threadPoolCreate(30,5,100);
    printf("111");
    for (int i = 0; i < 100; i++) {
        int* k = (int*)malloc(sizeof(int));
        *k = i + 100;
        threadpooladd(t,TaskFuntion,k);
    }

    sleep(30);
    threadPoolDestroy(t);
    return 0;
}