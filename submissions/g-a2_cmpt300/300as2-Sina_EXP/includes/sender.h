#ifndef SENDER_H
#define SENDER_H

#include "sender.h"
#include "sockets.h"
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

// destroys the list mutex for the sending
void destroySenderMutex();

// Initializes the IP and threads for sender
void initSender(char *sport, char *rcpu, pthread_t input, pthread_t sender);

// Scans the input stream for data followed by enter, and adds null-terminated string to list ADT
void *awaitInput(void *sharedList);

// Transmitting end of the socket which processes list and transmits the value in FIFO order
void *sendThread(void *unused);

#endif