#include "sendUDPThread.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "listHandler.h"
#include "socketHandler.h"

// Make threadPID static to be able to access it in different methods
static pthread_t sendUDPThreadPID;

void sendUDPThread_init(void) {
    printf("Inside sendUDPThread - INIT\n");
    pthread_create(
        &sendUDPThreadPID,  // PID (by pointer)
        NULL,               // Atrributes
        sendData,           // Function
        NULL);              // Arguments
}

void* sendData(void* unused) {
    printf("Inside sendUDPThread\n");

    int localSocketDescriptor = getLocalSocketValue();
    struct addrinfo* remoteAddressInfo = getRemoteMachineAddress();
    while (1) {
        char* sendData = (char*)removeFromKdbAndSendList();
        // printf("Inside sendUDPThread and sending >> %s\n", sendData);
        unsigned int dataLength = strlen(sendData);
        if (sendto(localSocketDescriptor, sendData, dataLength + 1, 0, remoteAddressInfo->ai_addr, remoteAddressInfo->ai_addrlen) == -1) {
            printf("ERROR OCCURED WHILE SENDING DATA\n");
            free(sendData);
            shutDownSocket();
            break;
        }

        // check for user input close condition of entering !
        if (*(sendData) == '!' && strlen(sendData) == 2) {
            printf("User intiated shutdown ... Closing session now\n");
            free(sendData);
            shutDownSocket();
            break;
        }
        free(sendData);
    }
    return NULL;
}

void sendUDPThread_shutdown(void) {
    // Cancel thread
    pthread_cancel(sendUDPThreadPID);

    // Waits for thread to finish
    pthread_join(sendUDPThreadPID, NULL);
}