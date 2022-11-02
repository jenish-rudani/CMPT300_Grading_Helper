#include <stdio.h>
#include "general.h"
#include "string.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "netdb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "string.h"
#include "unistd.h"
#include "signal.h"
#include "receiver.h"
#include "list.h"
#include "general.h"

static char* ip;
static int port;
static List* list;
static int* isProgramFinished;
static int socketDescriptor;       // Make socket for UDP
static char* messageTx;

static pthread_t threadPID;
static pthread_cond_t* syncItemAvailable;
static pthread_cond_t* syncBufAvailable;
static pthread_mutex_t* syncOkToPrintMutex;

void* Send_thread(void* unused){
    printf("Connecting to: %s:%d\n", ip, port);
    fflush(stdout);
    
    /* Setup Socket */
    struct sockaddr_in sin;     // Address


    if ( (socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0)) < 0 ){
        perror("socket creation failed");
        exit(EXIT_FAILURE); 
    }

    memset(&sin, 0, sizeof(sin));    

    sin.sin_family = AF_INET;                                           // Connection could be from network
    sin.sin_port = htons(port);                                         // Network to Host short
    sin.sin_addr.s_addr = inet_pton(AF_INET, ip, &(sin.sin_addr));      // Network to Host long

    /* Handle Sending */

    while (1) {
        // Critical Section
        pthread_mutex_lock(syncOkToPrintMutex);
            if(List_count(list) == 0 || List_count(list) == 1){
                pthread_cond_wait(syncItemAvailable, syncOkToPrintMutex);
            }

            messageTx = (char *)List_curr(list);
            sendto(socketDescriptor, messageTx, strlen(messageTx), 0, (struct sockaddr *) &sin, sizeof(sin));

            // Initiates program end
            if (messageTx[0] == '!' && (messageTx[1] == '\0' || messageTx[1] == '\n')){
                (*isProgramFinished)++;
                free(messageTx);
                break;
            }

            List_trim(list);
            free(messageTx);

            pthread_cond_signal(syncBufAvailable);
        pthread_mutex_unlock(syncOkToPrintMutex);                   
    }
    
    return 0;
}

void Sender_init(List* sendList, pthread_cond_t* syncSendItemAvailable, pthread_cond_t* syncSendBufAvailable, pthread_mutex_t* syncSendOkToPrintMutex, int sendPort, char* sendIP, int* programFinished){
    ip = sendIP;
    port = sendPort;
    list = sendList;
    isProgramFinished = programFinished;

    syncItemAvailable = syncSendItemAvailable;
    syncBufAvailable = syncSendBufAvailable;
    syncOkToPrintMutex = syncSendOkToPrintMutex;

    pthread_create(
        &threadPID,         // PID
        NULL,               // Attributes
        Send_thread,        // Function
        NULL);
}

void Sender_shutdown(){
    close(socketDescriptor);

    // Cancel Thread
    pthread_cancel(threadPID);
    
    // Waits for thread to finish
    pthread_join(threadPID, NULL);
}