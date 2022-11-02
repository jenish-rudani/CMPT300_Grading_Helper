#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "pthread.h"
#include "keyboard.h"
#include "list.h"
#include "general.h"

static List* list;
static char* messageRx;

static pthread_t threadPID;
static pthread_cond_t* syncItemAvailable;
static pthread_cond_t* syncBufAvailable;
static pthread_mutex_t* syncOkToPrintMutex;

void* Screen_thread(){
    while (1) {
        pthread_mutex_lock(syncOkToPrintMutex);
            if(List_count(list) == 0 || List_count(list) == 1){
                pthread_cond_wait(syncItemAvailable, syncOkToPrintMutex);
            }

            messageRx = (char *)List_curr(list);

            printf("\nMessage Received: \n'%s' \n", messageRx);
            List_trim(list);
            free(messageRx);

            pthread_cond_signal(syncBufAvailable);

        pthread_mutex_unlock(syncOkToPrintMutex);
    }

    return 0;
}

void Screen_init(List* receiveList, pthread_cond_t* syncRecItemAvailable, pthread_cond_t* syncRecBufAvailable, pthread_mutex_t* syncRecOkToPrintMutex){
    list = receiveList;

    syncItemAvailable = syncRecItemAvailable;
    syncBufAvailable = syncRecBufAvailable;
    syncOkToPrintMutex = syncRecOkToPrintMutex;

    pthread_create(
        &threadPID,         // PID
        NULL,               // Attributes
        Screen_thread,      // Function
        NULL);
}

void Screen_shutdown(){
    // Cancel Thread
    pthread_cancel(threadPID);
    // Waits for thread to finish
    pthread_join(threadPID, NULL);
}