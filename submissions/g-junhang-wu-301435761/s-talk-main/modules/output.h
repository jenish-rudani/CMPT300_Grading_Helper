#ifndef _MONITOR_OUTPUT_H_
#define _MONITOR_OUTPUT_H_

#include "../list/list.h"
#include <pthread.h>

void Output_init(List* outputLst, pthread_cond_t* bufAvail, pthread_cond_t* itemAvail, pthread_mutex_t* outputMutex);

void Output_shutdown();

void Output_cancel();

#endif