#ifndef _SENDER_H_
#define _SENDER_H_
#include "list.h"

void Sender_init(List*, pthread_cond_t*, pthread_cond_t*, pthread_mutex_t*, int, char*, int*);
void Sender_shutdown();

#endif