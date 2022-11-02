#include <stdio.h>
#include <stdlib.h> 
#include <netdb.h>
#include <string.h> 
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "recv.h"
#include "list.h"

#define MSG_MAX_LEN 256

//synchronization
pthread_mutex_t msgMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t okToPrint = PTHREAD_COND_INITIALIZER;
static int socketDescriptor;
static pthread_t pthreadPID;
static struct sockaddr_in* sinRemote;

// recv and screen list
List* msgRPList;

void* recvThread(void* arg) {
    
    while(1) {
        //receiving data
        unsigned int sin_len = sizeof(*sinRemote);
        char messageRecv[MSG_MAX_LEN];
        int bytesRecv = recvfrom(socketDescriptor, messageRecv, MSG_MAX_LEN, 0, (struct sockaddr*)sinRemote, &sin_len);

        //make it null terminated (so string functions work)
        int terminatedIdx = (bytesRecv < MSG_MAX_LEN) ? bytesRecv : MSG_MAX_LEN -1;
        messageRecv[terminatedIdx] = '\0';
        if (bytesRecv < 0) {
            printf("Error-from receiver");
            break;
        }
        List_prepend(msgRPList, messageRecv);
        pthread_mutex_lock(&msgMutex);
        {
            pthread_cond_signal(&okToPrint); // signal the screen.c that it can print
        }
        pthread_mutex_unlock(&msgMutex);
    
        if(strncmp(messageRecv, "!", 1) == 0 && strlen(messageRecv) == 2) {
            printf("s-talk session is termiated...\n");
            exit(0);
        }
    }

    return NULL;
}

void Recv_init(struct sockaddr_in* s, int sd) {
    sinRemote = s;
    socketDescriptor = sd;
    msgRPList = List_create();
    pthread_create(
        &pthreadPID,  //PID (by pointer)
        NULL,
        recvThread, //function
        NULL);
}

void Recv_shutDown() {
    //wait for thread to finish
    pthread_join(pthreadPID, NULL);

}