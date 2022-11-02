#ifndef _UDP_RECEIVER_H_
#define _UDP_RECEIVER_H_

#include <netdb.h>
#include <pthread.h>
#include "../list/list.h"

void Receiver_init(struct sockaddr_in* sinRemote, int socketDescriptor, List* outputLst,
                    pthread_cond_t* bufAvail, pthread_cond_t* itemAvail, pthread_mutex_t* outputMutex);

void Receiver_shutdown();

#endif