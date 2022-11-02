#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "output.h"
#include "../manager/shutdownManager.h"

static pthread_t s_outputID;
static List* s_lst;
static char* msg;

// synchronization
static pthread_cond_t* s_bufAvail;
static pthread_cond_t* s_itemAvail;
static pthread_mutex_t* s_outputMutex;

// printing messages
void* Output_thread(void* arg) {
    while (1) {
        // if the output list is empty, wait for receiver thread to receive message from remote socket port
        if (List_count(s_lst) == 0) {
            pthread_mutex_lock(s_outputMutex);
            {
                pthread_cond_wait(s_itemAvail, s_outputMutex);
            }
            pthread_mutex_unlock(s_outputMutex);
        }

        // remove the last message in the output list, since the message in prepended, removing the last message is FIFO
        // so that the first message received from receive thread gets to print first
        msg = List_trim(s_lst);

        // signal receive thread that there is some space in the output list
        pthread_mutex_lock(s_outputMutex);
        {
            pthread_cond_signal(s_bufAvail);
        }
        pthread_mutex_unlock(s_outputMutex);

        // print the message to screen
        puts(msg);

        // if msg was only the '!', trigger shutdown
        if (msg[0] == '!' && strlen(msg) == 1) {
            TriggerShutdown();
        }

        free(msg);
        msg = NULL;
    }
    
    return(NULL);
}

void Output_init(List* inputLst, pthread_cond_t* bufAvail, pthread_cond_t* itemAvail, pthread_mutex_t* outputMutex) {
    // get parameters
    s_lst = inputLst;
    s_bufAvail = bufAvail;
    s_itemAvail = itemAvail;
    s_outputMutex = outputMutex;
    
    if (pthread_create(&s_outputID, NULL, Output_thread, NULL) != 0) {
        exit(1);
    }
}

void Output_shutdown() {
    if (pthread_cancel(s_outputID) != 0) {
        exit(1);
    }

    if (pthread_join(s_outputID, NULL) != 0) {
        exit(1);
    }

    free(msg);
    msg = NULL;
    pthread_mutex_destroy(s_outputMutex);
    pthread_cond_destroy(s_bufAvail);
    pthread_cond_destroy(s_itemAvail);
}