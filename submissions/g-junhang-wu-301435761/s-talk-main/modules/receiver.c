#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "receiver.h"
#include "output.h"

#define MAX_LEN 256 // size of msg
#define MAX_SIZE 100 // size of the list

static struct sockaddr_in* s_sinRemote;
static int s_socketDescriptor;
static pthread_t s_receiverID;
static List* s_lst;
static char* msg;

// synchronziation
static pthread_cond_t* s_bufAvail;
static pthread_cond_t* s_itemAvail;
static pthread_mutex_t* s_outputMutex;

// receiving messages
void* Receiver_thread(void* arg) {
    while(1) {
        // receive message from remote socket port via UDP
        unsigned int sinLen = sizeof(*s_sinRemote);
        msg = malloc(MAX_LEN);

        int bytesRx = recvfrom(s_socketDescriptor,
                           msg, MAX_LEN, 0,
                           (struct sockaddr*) s_sinRemote, &sinLen);

        // adding the null terminating value to the end of the string
        int terminateIndex = (bytesRx < MAX_LEN) ? bytesRx : MAX_LEN - 1;
        msg[terminateIndex] = '\0';
        
        // if the output list is full, wait until there is some space
        if (List_count(s_lst) == MAX_SIZE) {
            pthread_mutex_lock(s_outputMutex);
            { 
                pthread_cond_wait(s_bufAvail, s_outputMutex);
            }
            pthread_mutex_unlock(s_outputMutex);
        }

        List_prepend(s_lst, msg); // prepend the message to the output list

        // signal output thread that there is some message in the output list
        pthread_mutex_lock(s_outputMutex);
        {
            pthread_cond_signal(s_itemAvail);
        }
        pthread_mutex_unlock(s_outputMutex);
    }

    return(NULL);
}

void Receiver_init(struct sockaddr_in* sinRemote, int socketDescriptor, List* outputLst, pthread_cond_t* bufAvail, pthread_cond_t* itemAvail, pthread_mutex_t* outputMutex) {
    // get parameters
    s_sinRemote = sinRemote;
    s_socketDescriptor = socketDescriptor;
    s_lst = outputLst;
    s_bufAvail = bufAvail;
    s_itemAvail = itemAvail;
    s_outputMutex = outputMutex;

    if (pthread_create(&s_receiverID, NULL, Receiver_thread, NULL) != 0) {
        exit(0);
    }
}

void Receiver_shutdown() {
    if (pthread_cancel(s_receiverID) != 0) {
        exit(1);
    }

    if (pthread_join(s_receiverID, NULL) != 0) {
        exit(1);
    }
}