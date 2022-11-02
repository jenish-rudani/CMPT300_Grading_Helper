#ifndef _RECEIVER_H_
#define _RECEIVER_H_
#include "list.h"
#include "pthread.h"

void Receiver_init(List*, pthread_cond_t*, pthread_cond_t*, pthread_mutex_t*, int, int*);
void Receiver_shutdown();

#endif