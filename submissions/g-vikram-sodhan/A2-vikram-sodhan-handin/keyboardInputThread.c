#include "keyboardInputThread.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "listHandler.h"

// Make threadPID static to be able to access it in different methods
static pthread_t keyboardInputThreadPID;

static char* dynamicMemoryMessage;

void keyboardInputThread_init() {
    printf("Inside keyboardInputThread - INIT\n");
    pthread_create(
        &keyboardInputThreadPID,  // PID (by pointer)
        NULL,                     // Atrributes
        keyboardInput,            // Function
        NULL);                    // Arguments

    // Waits for thread to finish
    pthread_join(keyboardInputThreadPID, NULL);
}

void* keyboardInput(void* unused) {
    printf("Inside keyboardInputThread\n");
    int size = 1000;
    char userInput[1000];
    while (1) {
        if (fgets(userInput, size, stdin) != NULL) {
            // printf("User Inputted %s\n", userInput);
            dynamicMemoryMessage = (char*)malloc(size);
            strncpy(dynamicMemoryMessage, userInput, size);
            addToKdbAndSendList(dynamicMemoryMessage);
        }
    }
    return NULL;
}

void keyboardInputThread_shutdown(void) {
    // Cancel thread
    pthread_cancel(keyboardInputThreadPID);

    // Waits for thread to finish
    pthread_join(keyboardInputThreadPID, NULL);

    free(dynamicMemoryMessage);
}