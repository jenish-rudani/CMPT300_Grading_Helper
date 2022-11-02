#ifndef _THREAD_H_
#define _THREAD_H_
#include <stdbool.h>

#define NUM_THREAD 4
#define MAX_MSG_LENGTH 256

void *receiveKeyboardInput(void* joe);
void *sendUDPMessage(void* joe);
void *receiveUDPMessage(void* joe);
void *outputToScreen(void* joe);
void joinAllThreads();
void createAllThreads();
void shutdownProgram();

#endif