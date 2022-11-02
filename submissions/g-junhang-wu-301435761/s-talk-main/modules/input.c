#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"

#define MAX_LEN 256 // size of msg
#define MAX_SIZE 100 // size of the list
 
static pthread_t s_inputID;
static List* s_lst;
static char* msg;

// synchronization
static pthread_cond_t* s_bufAvail;
static pthread_cond_t* s_itemAvail;
static pthread_mutex_t* s_inputMutex;

// reading messages
void* Input_thread(void* arg) {
    while (1) {
        // read message from keyboard using fgets
        msg = malloc(MAX_LEN);
        size_t length = -1;
        while (length <= 0 || length > MAX_LEN-1) {
            fgets(msg, MAX_LEN, stdin);
            length = strlen(msg) - 1;
        }

        if (msg[length] == '\n') {
            msg[length] = '\0';
        }

        // is the input list is full, wait until there is some space
        if (List_count(s_lst) == MAX_SIZE) {
            pthread_mutex_lock(s_inputMutex);
            { 
                pthread_cond_wait(s_bufAvail, s_inputMutex);
            }
            pthread_mutex_unlock(s_inputMutex);
        }

        List_prepend(s_lst, msg); // prepend the message to the input list

        // signal sender thread that there is some message in the input list
        pthread_mutex_lock(s_inputMutex);
        {
            pthread_cond_signal(s_itemAvail);
        }
        pthread_mutex_unlock(s_inputMutex);
    }

    return(NULL);
}

void Input_init(List* inputLst, pthread_cond_t* bufAvail, pthread_cond_t* itemAvail, pthread_mutex_t* inputMutex) {
    // get parameters
    s_lst = inputLst;
    s_bufAvail = bufAvail;
    s_itemAvail = itemAvail;
    s_inputMutex = inputMutex;

    if (pthread_create(&s_inputID , NULL, Input_thread, NULL) != 0) {
        exit(1);
    }
}

void Input_shutdown() {
    if (pthread_cancel(s_inputID) != 0) {
        exit(1);
    }
    
    if (pthread_join(s_inputID, NULL) !=0) {
        exit(1);
    }
}
