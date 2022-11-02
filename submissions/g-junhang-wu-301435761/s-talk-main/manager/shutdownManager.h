#ifndef _SHUTDOWN_MANAGER_H_
#define _SHUTDOWN_MANAGER_H_

#include <pthread.h>

void WaitForShutDown(pthread_cond_t* shutdownCondVar, pthread_mutex_t* shutdownMutex);

void TriggerShutdown();

#endif