#ifndef THREADS_H
#define THREADS_H
#include <pthread.h>

void initMutexesAndConditions();
void initThreads(pthread_t thread1, pthread_t thread2, pthread_t thread3, pthread_t thread4);
void * getMessage(void * outgoingList);
void * dispatchMessage(void * outgoingList);
void * getMessageFromNetwork(void * incomingList);
void * printMessageOnScreen(void * incomingList);
void  cancelAllThreads();
#endif