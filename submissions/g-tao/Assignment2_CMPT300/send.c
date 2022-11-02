#include <stdio.h>
#include <stdlib.h> 
#include <netdb.h>
#include <string.h> 
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "list.h"
#include "send.h"

#define MSG_MAX_LEN 256


static pthread_t pthreadPID;
//synchronization
extern pthread_mutex_t inMutex;
extern pthread_cond_t okToSend;
// send and keyboard list
extern List* inKSList;
static struct sockaddr_in* sin;
static int socketDescriptor;
static char *messageSend;
void* sendThread(void* arg) {
    
    while(1) {
        // sending data
        if (List_count(inKSList) == 0) {
        pthread_mutex_lock(&inMutex);
        {
        pthread_cond_wait(&okToSend, &inMutex); //wait for signal if no input
        }
        pthread_mutex_unlock(&inMutex);
        }  
        messageSend = List_trim(inKSList);
        // transmit a reply
        int sin_len = sizeof(*sin);
        int bytesSend = sendto(socketDescriptor, messageSend, strlen(messageSend), 0, (struct sockaddr*)sin, sin_len);
        if(bytesSend < 0) {
            printf("Error-from Sender");
        }
       if(strncmp(messageSend, "!", 1) == 0 && strlen(messageSend) == 2) {
            printf("s-talk session is termiated...\n");
            exit(0);
        }

    }
       
}

void Send_init(struct sockaddr_in* si, int sd) {
    sin = si; 
    socketDescriptor = sd;
    pthread_create(
    &pthreadPID,  //PID (by pointer)
    NULL,
    sendThread, //function
    NULL);
}
void Send_shutDown() {
    //wait for thread to finish
    pthread_join(pthreadPID, NULL);
    //free messageSend
    free(messageSend); 
}