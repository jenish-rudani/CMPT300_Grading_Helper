#include <stdio.h>
#include <stdlib.h> 
#include <netdb.h>
#include <string.h> 
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "list.h"
#include "keyboard.h"
#include "send.h"
#include "recv.h"
#include "screen.h"

#define MSG_MAX_LEN 256
//synchronization
pthread_mutex_t inMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t okToSend = PTHREAD_COND_INITIALIZER;
// send and keyboard list
List* inKSList;
static pthread_t pthreadPID;

void* keyBoardThread(void* arg) {
    while(1) {
        char inputKeyBoard[MSG_MAX_LEN];
        fgets(inputKeyBoard, MSG_MAX_LEN, stdin);
       
        pthread_mutex_lock(&inMutex);
        {
            pthread_cond_signal(&okToSend); //signal the send.c that it can send
        }
        
        pthread_mutex_unlock(&inMutex);
         List_prepend(inKSList, inputKeyBoard);
    }
    return NULL;
}

void Keyboard_init() {
  
    inKSList = List_create();
    pthread_create(
        &pthreadPID,  //PID (by pointer)
        NULL,
        keyBoardThread, //function
        NULL);
}

void Keyboard_shutDown() {
    //cancel thread 
    pthread_cancel(pthreadPID);
    //wait for thread to finish
    pthread_join(pthreadPID, NULL);
}
