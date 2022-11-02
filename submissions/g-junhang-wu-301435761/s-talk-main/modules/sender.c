#include <stdlib.h>
#include <string.h>
#include "sender.h"
#include "input.h"
#include "../manager/shutdownManager.h"

static struct sockaddr_in* s_sinRemote;
static int s_socketDescriptor;
static pthread_t s_senderID;
static List* s_lst;
static char* msg; 

// synchronziation
static pthread_cond_t* s_bufAvail;
static pthread_cond_t* s_itemAvail;
static pthread_mutex_t* s_inputMutex;

// sending messages
void* Sender_thread(void* arg) {
    while (1) {
        // if the intput list is empty, wait for user inputs a message
        if (List_count(s_lst) == 0) {
            pthread_mutex_lock(s_inputMutex);
            {   
                pthread_cond_wait(s_itemAvail, s_inputMutex);
            }
            pthread_mutex_unlock(s_inputMutex);
        }

        // remove the last message in the input list, since the message in prepended, removing the last message is FIFO
        // so that the first message the user types gets send first
        msg = List_trim(s_lst);

        // signal input thread that there is some space in the input list
        pthread_mutex_lock(s_inputMutex);
        {
            pthread_cond_signal(s_bufAvail);
        }
        pthread_mutex_unlock(s_inputMutex);

        // send the message via UDP to remote socket port
        int sinLen = sizeof(*s_sinRemote);
        sendto(s_socketDescriptor,
            msg, strlen(msg), 0,
            (struct sockaddr *)s_sinRemote, sinLen);

        // if msg was only the '!', trigger shutdown
        if (msg[0] == '!' && strlen(msg) == 1) {
            TriggerShutdown();
        }
        
        free(msg);
        msg = NULL;
    }
   
    return(NULL);
}

void Sender_init(struct sockaddr_in* sinRemote, int socketDescriptor, List* inputLst, pthread_cond_t* bufAvail, pthread_cond_t* itemAvail, pthread_mutex_t* inputMutex) {
    // get parameters
    s_sinRemote = sinRemote;
    s_socketDescriptor = socketDescriptor;
    s_lst = inputLst;
    s_bufAvail = bufAvail;
    s_itemAvail = itemAvail;
    s_inputMutex = inputMutex;

    if (pthread_create(&s_senderID , NULL, Sender_thread, NULL) != 0) {
        exit(1);
    }
}

void Sender_shutdown() {    
    if (pthread_cancel(s_senderID) != 0) {
        exit(1);
    }
    
    if (pthread_join(s_senderID, NULL) != 0) {
        exit(1);
    }

    free(msg);
    msg = NULL;
    pthread_mutex_destroy(s_inputMutex);
    pthread_cond_destroy(s_bufAvail);
    pthread_cond_destroy(s_itemAvail);
}