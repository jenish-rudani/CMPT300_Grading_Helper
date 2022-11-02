#include "shutdownManager.h"

static pthread_cond_t* s_shutdownCondVar;
static pthread_mutex_t* s_shutdownMutex;

// make the calling process wait for shutdown
void WaitForShutDown(pthread_cond_t* shutdownCondVar, pthread_mutex_t* shutdownMutex) {
    s_shutdownCondVar = shutdownCondVar;
    s_shutdownMutex = shutdownMutex;

    // make the main process wait for shutdown so it does not call Thread_shutdown()
    pthread_mutex_lock(s_shutdownMutex);
    {
        pthread_cond_wait(s_shutdownCondVar, s_shutdownMutex);
    }
    pthread_mutex_unlock(s_shutdownMutex);
}

void TriggerShutdown() {
    // once the threads signal for shutdown, unblock the process blocked by s_shutdownCondVar(main process)
    pthread_mutex_lock(s_shutdownMutex);
    {
        pthread_cond_signal(s_shutdownCondVar);
    }
    pthread_mutex_unlock(s_shutdownMutex);
}
