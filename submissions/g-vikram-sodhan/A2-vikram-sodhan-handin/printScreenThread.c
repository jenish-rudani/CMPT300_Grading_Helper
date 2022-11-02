#include "printScreenThread.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "listHandler.h"

// Make threadPID static to be able to access it in different methods
static pthread_t printScreenThreadPID;

void printScreenThread_init(void) {
    // printf("Inside printScreenThread- INIT\n");
    pthread_create(
        &printScreenThreadPID,  // PID (by pointer)
        NULL,                   // Atrributes
        printToScreen,          // Function
        NULL);                  // Arguments
}

void* printToScreen(void* unused) {
    printf("Inside printScreenThread\n");
    while (1) {
        char* screenData = (char*)removeFromReceiveAndPrintListList();
        // printf("Inside printScreenThread and printing >> %s\n", screenData);
        fputs(screenData, stdout);
        free(screenData);
    }
    return NULL;
}

void printScreenThread_shutdown(void) {
    // Cancel thread
    pthread_cancel(printScreenThreadPID);

    // Waits for thread to finish
    pthread_join(printScreenThreadPID, NULL);
}