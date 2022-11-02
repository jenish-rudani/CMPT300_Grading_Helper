#include <stdio.h>
#include <stdlib.h> 
#include <netdb.h>
#include <string.h> 
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "screen.h"
#include "list.h"

static pthread_t pthreadPID;
//synchronization
extern pthread_mutex_t msgMutex;
extern pthread_cond_t okToPrint;
// recv and screen list
extern List* msgRPList;
static char* print;
void* screenThread(void* arg) {
    //print the data
    while(1) {
        if (List_count(msgRPList) == 0){
        pthread_mutex_lock(&msgMutex);
        {
        pthread_cond_wait(&okToPrint, &msgMutex); // wait for signal if nothing to print 
        }
        pthread_mutex_unlock(&msgMutex);
        }
        
        print = List_trim(msgRPList);
        printf("message recevied: ");
        puts(print);
       
    }
    return NULL;
}
void Screen_init() {
    pthread_create(
        &pthreadPID,  //PID (by pointer)
        NULL,
        screenThread, //function
        NULL);
}
void Screen_shutDown() {
    //cancel thread 
    pthread_cancel(pthreadPID);
    //wait for thread to finish
    pthread_join(pthreadPID, NULL);
}