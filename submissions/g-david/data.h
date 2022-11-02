#ifndef _DATA_H_
#define _DATA_H_
#define MSG_MAX 256

#include <pthread.h>

#include "list.h"

// Data to send
struct send_data
{
    List *sendList;
    char *port;
    pthread_mutex_t sendListMutex;
    pthread_cond_t okToSendCondVar;
    int flag;
    char *ip;
};

// Data to receive
struct receive_data
{
    List *receiveList;
    char *port;
    pthread_mutex_t receiveListMutex;
    pthread_cond_t okToReceiveCondVar;
    int flag;
    char *ip;
};

#endif