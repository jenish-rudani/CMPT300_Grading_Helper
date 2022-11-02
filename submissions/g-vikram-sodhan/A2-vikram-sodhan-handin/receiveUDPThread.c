#include "receiveUDPThread.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "listHandler.h"
#include "socketHandler.h"

// Make threadPID static to be able to access it in different methods
static pthread_t receiveUDPThreadPID;

static char* dynamicMemoryMessage;

void receiveUDPThread_init(void) {
    printf("Inside receiveUDPThread - INIT\n");
    pthread_create(
        &receiveUDPThreadPID,  // PID (by pointer)
        NULL,                  // Atrributes
        receiveData,           // Function
        NULL);                 // Arguments
}

void* receiveData(void* unused) {
    printf("Inside receiveUDPThread\n");

    int size = 1000;
    char inputBuffer[size];
    struct sockaddr_in receiverMachineInfo;
    unsigned int receiverLength = sizeof(receiverMachineInfo);
    int localSocketDescriptor = getLocalSocketValue();

    while (1) {
        int numbytes = recvfrom(localSocketDescriptor, inputBuffer, size - 1, 0, (struct sockaddr*)&receiverMachineInfo, &receiverLength);
        // check if remote user entered close condition of entering !
        if (*(inputBuffer) == '!' && strlen(inputBuffer) == 2) {
            printf("Received shutdown from remote machine ... Closing session now\n");
            shutDownSocket();
            break;
        }

        dynamicMemoryMessage = (char*)malloc(numbytes);
        strcpy(dynamicMemoryMessage, inputBuffer);

        // printf("Inside receiverUDPThread and received >> %s\n", dynamicMemoryMessage);
        addToReceiveAndPrintList(dynamicMemoryMessage);
    }
    return NULL;
}

void receiveUDPThread_shutdown(void) {
    // Cancel thread
    pthread_cancel(receiveUDPThreadPID);

    // Waits for thread to finish
    pthread_join(receiveUDPThreadPID, NULL);

    free(dynamicMemoryMessage);
}