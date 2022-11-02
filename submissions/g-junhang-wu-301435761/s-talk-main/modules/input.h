#ifndef _KEYBOARD_INPUT_H_
#define _KEYBOARD_INPUT_H_

#include "../list/list.h"
#include <pthread.h>

void Input_init(List* inputLst, pthread_cond_t* bufAvail, pthread_cond_t* itemAvail, pthread_mutex_t* inputMutex);

void Input_shutdown();

void Input_cancel();

#endif