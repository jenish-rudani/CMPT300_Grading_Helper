#ifndef RECEIVER_H
#define RECEIVER_H
#include "receiver.h"
#include "list.h"
#include <pthread.h>
#include <stdio.h>
#include "arpa/inet.h"
#include <stdlib.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

// Destroyes the list mutexes
void destroyReceiverMutex();

// Initializes the IP and threads for receiver
void initReceiver(char *rport, pthread_t output, pthread_t receive);

// Output method that continuously prints to the screen the data it is receiving
void *printMessage(void *sharedList);

// Receiving end of the socket which processes input and adds to local list
void *receiveThread(void *unused);

#endif