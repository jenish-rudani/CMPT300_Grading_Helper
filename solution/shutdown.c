#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "shutdown.h"

const char* SHUTDOWN_MESSAGE = "!\x0A";

static bool s_isShuttingDown = false;
static pthread_mutex_t s_shutdownMutex = PTHREAD_MUTEX_INITIALIZER;

void Shutdown_init()
{
    // Lock the mutex, and don't unlock it!
    pthread_mutex_lock(&s_shutdownMutex);
}

void Shutdown_waitForShutdown()
{
    pthread_mutex_lock(&s_shutdownMutex);
    pthread_mutex_unlock(&s_shutdownMutex);
}
void Shutdown_triggerShutdown()
{
    s_isShuttingDown = true;
    pthread_mutex_unlock(&s_shutdownMutex);
}
bool Shutdown_inShuttingDwon()
{
    return s_isShuttingDown;
}

void exitOnFail(bool isSuccessful, char* msg) 
{
    if (!isSuccessful) {
        fprintf(stderr, "ERROR: %s\n", msg);
        exit(EXIT_FAILURE);
    }
}
