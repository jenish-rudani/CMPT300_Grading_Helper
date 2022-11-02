// TODO: Code cleanup idea
//      This code is very very similar to the send module; could they be combined
//      and just override the behaviour for what to do when data is pulled
//      out of the list to process? Would need to stuff statics into a struct.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "list.h"
#include "shutdown.h"
#include "screen.h"

static pthread_t thread;
static List *s_pList;

static pthread_cond_t s_listCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_listMutex = PTHREAD_MUTEX_INITIALIZER;


static void* screenThread(void* unused) 
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
        }
        pthread_mutex_unlock(&s_listMutex);

        // exit?
        int msgLength = strlen(pMessage);
        if (msgLength == strlen(SHUTDOWN_MESSAGE) && strcmp(pMessage, SHUTDOWN_MESSAGE) == 0) {
            // printf("DONE (from screen): '%s'", pMessage);
            done = true;
        }

        // print
        fputs(pMessage, stdout);

        // cleanup
        free(pMessage);
    }

    // Only get here if we have received the shutdown message
    Shutdown_triggerShutdown();
    return NULL;
}


void Screen_init()
{
    pthread_mutex_lock(&s_listMutex);
    {
        // create list
        exitOnFail(
            (s_pList = List_create()) != NULL,
            "Unable to create screen list");

        // create thread
        exitOnFail(
            pthread_create(&thread, NULL, screenThread, NULL) == 0,
            "Unable to create screen thread");
    }
    pthread_mutex_unlock(&s_listMutex);    

}


static void freeListItems(void* pItem)
{
    printf("Freeing an unused item in the screen list: %s\n", (char*)pItem);
    free(pItem);
}

void Screen_shutdown()
{
    // cleanup thread
    // ..cancel may fail if thread already exited
    // NOTE: cancelling a thread blocked on a condition variable
    //       leaves its mutex locked (acquired)
    pthread_cancel(thread);
    exitOnFail(
        pthread_join(thread, NULL) == 0, 
        "Unable to join screen thread");

    // delete the list
    // don't re-acquire the mutex because it will be locked if
    // we cancelled the thread.
    List_free(s_pList, freeListItems);
    s_pList = NULL;
}

void Screen_printMessage(char *pMessage)
{
    pthread_mutex_lock(&s_listMutex);
    {
        // if not yet up, or shutting down, don't queue
        if (s_pList != NULL) {
            List_append(s_pList, pMessage);
            pthread_cond_signal(&s_listCond);
        }
    }
    pthread_mutex_unlock(&s_listMutex);
}
