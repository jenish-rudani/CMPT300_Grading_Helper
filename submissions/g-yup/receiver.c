#include "sender.h"
#include "keyboard.h"
#include "printer.h"
#include "receiver.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>

#define MSG_MAX_LEN 100

static List* pList;
static pthread_t threadReceiver;
static pthread_mutex_t listMutex = PTHREAD_MUTEX_INITIALIZER;
static char *message;
static char *homePortNumber;
static int socketDescriptor;

static void *receiverThread(void *unused)
{
    // Address
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;                // Connection may be from network
    sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to Network long
    sin.sin_port = htons(atoi(homePortNumber));     // Host to Network short

    // Create the socket for UDP
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

    // Bind the socket to the port (PORT) that we specify
    bind(socketDescriptor, (struct sockaddr *)&sin, sizeof(sin));

    while (1)
    {
        // Get the data(blocking : waiting until receiving data)
        // Will change sin(the address) to be the address of the client.
        // Note: sin passes information in and out of call!
        struct sockaddr_in sinRemote;
        unsigned int sin_len = sizeof(sinRemote);
        int bytesRx = recvfrom(socketDescriptor, message, MSG_MAX_LEN, 0, (struct sockaddr*) &sinRemote, &sin_len);

        // Make it null terminated (so string functions work):
        int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx : MSG_MAX_LEN-1;
        message[terminateIdx] = 0;
        
        // Add message
        Receiver_addMessageInList(message);

        //
        if(!strcmp(message, "!\n")) {
            // signal to writer && cancel keyboard and sender
            Printer_signal();
            Keyboard_cancel();
            Sender_cancel();
            return NULL;
        }
        
        Printer_signal();
    }
    return NULL;
}


void Receiver_init(char* portNumber) {
    pList = List_create();
    message = (char*) malloc(MSG_MAX_LEN*sizeof(char));
    homePortNumber = portNumber;
    pthread_create(&threadReceiver, NULL, receiverThread, NULL);
}

void Receiver_addMessageInList(char* message) {
    pthread_mutex_lock(&listMutex);
    {
        List_append(pList, message);
    }
    pthread_mutex_unlock(&listMutex);
}

char* Receiver_pickMessageInList() {
    char* message;
    pthread_mutex_lock(&listMutex);
    {
        message = List_trim(pList);
    }
    pthread_mutex_unlock(&listMutex);
    return message;
}

void Receiver_cancel() {
    pthread_cancel(threadReceiver);
}

void Receiver_waitForShutdown() {
    pthread_join(threadReceiver, NULL);

    // close socket 
    close(socketDescriptor);
    
    //Clean up
    free(message);
    message = NULL;
    List_free(pList, free);
}