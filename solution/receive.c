#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

#include "receive.h"
#include "screen.h"
#include "shutdown.h"
#include "socket.h"

#define MAX_SIZE 512
static pthread_t thread;

static void* receiveThread(void* unused) 
{
    char buff[MAX_SIZE];
    while(true) {
        int bytesRx = Socket_receiveBlocking(buff, MAX_SIZE);

        // copy data; screen must free
        int length = bytesRx + 1;
        char *pMessageToQueue = malloc(length);
        strncpy(pMessageToQueue, buff, length);
        Screen_printMessage(pMessageToQueue);
    }
    return NULL;
}

void Receive_init()
{
    exitOnFail(
        pthread_create(&thread, NULL, receiveThread, NULL) == 0,
        "Unable to create receive thread");

}
void Receive_shutdown()
{
    // pthread_cancel may fail if thread already exited
    pthread_cancel(thread);
    
    exitOnFail(
        pthread_join(thread, NULL) == 0, 
        "Unable to join receive thread");
}