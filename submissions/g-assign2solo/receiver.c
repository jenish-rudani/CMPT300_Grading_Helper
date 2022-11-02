#include <stdio.h>
#include "general.h"
#include "string.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "netdb.h"
#include "string.h"
#include "unistd.h"
#include "signal.h"
#include "receiver.h"
#include "list.h"
#include "general.h"

static int port;
static int socketDescriptor;
static List* list;
int* isProgramFinished;
char* messageRx;

static pthread_t threadPID;
static pthread_cond_t* syncItemAvailable;
static pthread_cond_t* syncBufAvailable;
static pthread_mutex_t* syncOkToPrintMutex;

void* Receive_thread(void* unused){
    printf("Hosting on port: %d\n", port);
    fflush(stdout);

    /* Setup Socket */

    // Address
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));    
    sin.sin_family = AF_INET;                   // Connection could be from network
    sin.sin_addr.s_addr = htonl(INADDR_ANY);    // Host to Network long 
    sin.sin_port = htons(port);                 // Host to Network short

    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0); // Make socket for UDP

    // Bind the socket to the port (PORT) that we specify
    bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));
    
    /* Handle Receiving */

    while (1) {
        // Get the data (blocking)
        // Will change sin (address) to the address of client
        // Note: sin passes info in and out of call
        struct sockaddr_in sinRemote;               // Data from other connection
        unsigned int sin_len = sizeof(sinRemote);   // Size of data from other connection

        messageRx = (char *) malloc(MSG_MAX_LEN);
        memset(messageRx, 0, MSG_MAX_LEN);

        recvfrom(socketDescriptor, messageRx, MSG_MAX_LEN, 0, (struct sockaddr *) &sinRemote, &sin_len);

        // Initiates program end
        if (messageRx[0] == '!' && (messageRx[1] == '\0' || messageRx[1] == '\n')){
            (*isProgramFinished)++;
            break;
        }

        pthread_mutex_lock(syncOkToPrintMutex);
            if(List_count(list) == LIST_MAX_NUM_NODES){
                pthread_cond_wait(syncBufAvailable, syncOkToPrintMutex);
            }
            List_prepend(list, messageRx);
            pthread_cond_signal(syncItemAvailable);
        pthread_mutex_unlock(syncOkToPrintMutex);
    }

    return 0;
}

void Receiver_init(List* receiveList, pthread_cond_t* syncRecItemAvailable, pthread_cond_t* syncRecBufAvailable, pthread_mutex_t* syncRecOkToPrintMutex, int receivingPort, int* programFinished){
    list = receiveList;
    port = receivingPort;
    isProgramFinished = programFinished;

    syncItemAvailable = syncRecItemAvailable;
    syncBufAvailable = syncRecBufAvailable;
    syncOkToPrintMutex = syncRecOkToPrintMutex;

    pthread_create(
        &threadPID,         // PID
        NULL,               // Attributes
        Receive_thread,     // Function
        NULL);
}

void Receiver_shutdown(){
    close(socketDescriptor);

    // Cancel Thread
    pthread_cancel(threadPID);
    
    // Waits for thread to finish
    pthread_join(threadPID, NULL);

    free(messageRx);
    messageRx = NULL;
}