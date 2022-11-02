#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "pthread.h"
#include "general.h"
#include "keyboard.h"
#include "sender.h"
#include "receiver.h"
#include "screen.h"
#include "list.h"

static pthread_t threadPID;
static List* list;
char* messageTx;

static pthread_t threadPID;
static pthread_cond_t* syncItemAvailable;
static pthread_cond_t* syncBufAvailable;
static pthread_mutex_t* syncOkToPrintMutex;

void* Keyboard_thread(void* unused){
    while(1){
        messageTx = (char *) malloc(MSG_MAX_LEN);
        memset(messageTx, 0, MSG_MAX_LEN);

        printf("Send a message (%d character limit, type ! to end program): \n", MSG_MAX_LEN-1);

        // Get message from user
        if (fgets(messageTx, MSG_MAX_LEN, stdin)){

            // Checks for empty messages
            if (messageTx[0] == '\0' || messageTx[0] == '\n') {
                printf("Error: Empty Message\n\n");
                free(messageTx);
                continue;
            }
            
            // Sends messages up to the max message limit
            // From: https://stackoverflow.com/questions/38767967/clear-input-buffer-after-fgets-in-c
            char *p;
                if((p = strchr(messageTx, '\n'))){      // check exist newline
                    *p = 0;
                } else {
                    scanf("%*[^\n]");scanf("%*c");      // clear upto newline
                    printf("Message trimmed to fit %d bytes: %s \n", MSG_MAX_LEN, messageTx);
            }
        }

        pthread_mutex_lock(syncOkToPrintMutex);
            if(List_count(list) == LIST_MAX_NUM_NODES){
                pthread_cond_wait(syncBufAvailable, syncOkToPrintMutex);
            }

            List_prepend(list, messageTx);
            pthread_cond_signal(syncItemAvailable);
        pthread_mutex_unlock(syncOkToPrintMutex);
    }

    return 0;
}

void Keyboard_init(List* sendList, pthread_cond_t* syncSendItemAvailable, pthread_cond_t* syncSendBufAvailable, pthread_mutex_t* syncSendOkToPrintMutex){
    list = sendList;
    
    syncItemAvailable = syncSendItemAvailable;
    syncBufAvailable = syncSendBufAvailable;
    syncOkToPrintMutex = syncSendOkToPrintMutex;

    pthread_create(
        &threadPID,         // PID
        NULL,               // Attributes
        Keyboard_thread,    // Function
        NULL);
}

void Keyboard_shutdown(){
    // Cancel Thread
    pthread_cancel(threadPID);
    
    // Waits for thread to finish
    pthread_join(threadPID, NULL);
    
    free(messageTx);
}