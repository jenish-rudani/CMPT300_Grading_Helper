#include <stdio.h>
#include "list.h"
#include <pthread.h>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>

#include<netdb.h>
#include<arpa/inet.h>

static struct sockaddr_in myReceiveAddr;
static struct sockaddr_in mySendAddr;
static struct sockaddr_in remoteReceiveAddr;
static struct sockaddr_in remoteSendAddr;
static int receiveSocket;
static int sendSocket;
static int myPort;
static int friendPort;
static char machineName[1024];
static char friendPortString[1024];





void* keyboardControl(void* sharedList);
void* sendControl(void* sharedList);
void* receiveControl(void * sharedList);
void* printControl(void* sharedList);
void* initThreads(void* unused);

void getPorts();

