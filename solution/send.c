#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "send.h"
#include "list.h"
#include "shutdown.h"
#include "socket.h"


static pthread_t thread;
static List *s_pList;

static pthread_cond_t s_listCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_listMutex = PTHREAD_MUTEX_INITIALIZER;


static void* sendThread(void* unused) 
{
    bool done = false;
    while(!done) {
        char* pMessage = NULL;

        // get data (wait if none)
        pthread_mutex_lock(&s_listMutex);
        {
            // Wait for data
            if (List_count(s_pList) == 0) {
                pthread_cond_wait(&s_listCond, &s_listMutex);
            }

            // Get the data
            List_first(s_pList);
            pMessage = List_remove(s_pList);
            exitOnFail(pMessage != NULL, "GOT NULL OUT OF LIST\n");
        }
        pthread_mutex_unlock(&s_listMutex);

        // exit?
        int msgLength = strlen(pMessage);
        if (msgLength == strlen(SHUTDOWN_MESSAGE) && strcmp(pMessage, SHUTDOWN_MESSAGE) == 0) {
            // printf("DONE (from Send): '%s'", pMessage);
            done = true;
        }

        // send over socket
        // printf("(SEND) %s", pMessage);
        exitOnFail(
            Socket_sendMessage(pMessage, msgLength) == msgLength,
            "Failed to send all the data");

        // cleanup
        free (pMessage);
    }

    // Only get here if we have received the shutdown message
    Shutdown_triggerShutdown();
    return NULL;
}


void Send_sendMessage(char *pMessage)
{
    pthread_mutex_lock(&s_listMutex);
    {
        // if not yet up, or shutting down, don't queue
        if (s_pList != NULL) {

            if (List_count(s_pList) > 50) {
                fprintf(stderr, "WARNING: List filling on apppend : %d = %s\n", List_count(s_pList), pMessage);
            }

            if (List_append(s_pList, pMessage) != LIST_SUCCESS) {
                fprintf(stderr, "APPEND FAIL; size %d, ptr 0x%ld msg = %s\n", List_count(s_pList), (long) pMessage, pMessage);    
                free(pMessage);
                pMessage = NULL;
            }
            pthread_cond_signal(&s_listCond);
        } else {
            fprintf(stderr, "ERROR Queing message to send while shutting down! (%s)\n", pMessage);
            free(pMessage);
            pMessage = NULL;
        }
    }
    pthread_mutex_unlock(&s_listMutex);
}


void Send_init()
{
    pthread_mutex_lock(&s_listMutex);
    {
        // create list
        exitOnFail(
            (s_pList = List_create()) != NULL,
            "Unable to create send list");

        // create thread
        exitOnFail(
            pthread_create(&thread, NULL, sendThread, NULL) == 0,
            "Unable to create send thread");
    }
    pthread_mutex_unlock(&s_listMutex);    

}


static void freeListItems(void* pItem)
{
    printf("Freeing an unused item in the send list: %s\n", (char*)pItem);
    free(pItem);
}

void Send_shutdown()
{
    // cleanup thread
    // ..cancel may fail if thread already exited
    // NOTE: cancelling a thread blocked on a condition variable
    //       leaves its mutex locked (acquired)
    pthread_cancel(thread);
    exitOnFail(
        pthread_join(thread, NULL) == 0, 
        "Unable to join send thread");

    // delete the list
    // don't re-acquire the mutex because it will be locked if
    // we cancelled the thread.
    List_free(s_pList, freeListItems);
    s_pList = NULL;
}
