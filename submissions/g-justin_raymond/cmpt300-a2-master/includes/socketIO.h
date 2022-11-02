#ifndef SOCKETIO_H_
#define SOCKETIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "list.h"

#define MAXMSGSIZE 256      // Used for send and receive

// Struct for the arguments needed for UDP_send() and UDP_recv()
typedef struct socketIN_s socketIN;
struct socketIN_s{
    List* data;
    int *socketfd;
    char **myPort;
    char **remotePort;
    char **remoteMachineName;
    bool *endFlag;
    pthread_mutex_t *mutex;
};

// ** IMPLEMENTATION OF SEND AND RECV IS ** //
/*
    Heavily inspired from Beej's Guide to Network Programming
    https://beej.us/guide/bgnet/html/
*/

// Used for receiving a message, input should be a (socketIN *) type.
// Will manipulate the data, with the output from the socket regarding the
// input socketfd. Socket initialized to be binded to myPort
// When receiving a "!\n", the whole session will exit.
void* UDP_recv(void *args);

// Used for sending a message, input should be a (socketIN *) type.
// Will input into the socket by trimming the data, the socket regarding the
// input socketfd. Socket is "connected" to one that uses the desired
// hostname (remoteMachineName), and port (remotePort).
void* UDP_send(void *args);

#endif