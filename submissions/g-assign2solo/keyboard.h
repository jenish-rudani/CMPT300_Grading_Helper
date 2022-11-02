#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_
#include "list.h"

void Keyboard_init(List*, pthread_cond_t*, pthread_cond_t*, pthread_mutex_t*);
void Keyboard_shutdown();

#endif