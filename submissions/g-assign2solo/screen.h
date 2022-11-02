#ifndef _SCREEN_H_
#define _SCREEN_H_
#include "list.h"
#include "pthread.h"

void Screen_init(List*, pthread_cond_t*, pthread_cond_t*, pthread_mutex_t*);
void Screen_shutdown();

#endif