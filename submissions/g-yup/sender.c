#include "sender.h"
#include "keyboard.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>

#define MSG_MAX_LEN 100
static pthread_t threadSender;
static char *message;
static char *reHostName;
static char *rePortNumber;
static int socketDescriptor;

// Synchronization
static pthread_cond_t s_syncOKToSendCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncOKToSendMutex = PTHREAD_MUTEX_INITIALIZER;

static void *senderThread(void *unused)
{
    // Address
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;                // Connection may be from network
    sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to Network long
    sin.sin_port = htons(atoi(rePortNumber));     // Host to Network short

    // Create the socket for UDP
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

    // Bind the socket to the port (PORT) that we specify
    bind(socketDescriptor, (struct sockaddr *)&sin, sizeof(sin));

    while (1)
    {
        // Get the data(blocking : waiting until receiving data)
        // Will change sin(the address) to be the address of the client.
        // Note: sin passes information in and out of call!

        pthread_mutex_lock(&s_syncOKToSendMutex);
        {
            pthread_cond_wait(&s_syncOKToSendCondVar, &s_syncOKToSendMutex);
        }
        pthread_mutex_unlock(&s_syncOKToSendMutex);

        message = Keyboard_pickMessageInList();

        sendto(socketDescriptor, message, MSG_MAX_LEN, 0, (struct sockaddr *)&sin, sizeof(sin));

        if(!strcmp(message, "!\n")) {
            return NULL;
        }

    }
    return NULL;
}

void Sender_init(char *hostName, char *port)
{
    reHostName = hostName;
    rePortNumber = port;

    pthread_create(&threadSender, NULL, senderThread, NULL);
}

void Sender_signal()
{
    pthread_mutex_lock(&s_syncOKToSendMutex);
    {
        pthread_cond_signal(&s_syncOKToSendCondVar);
    }
    pthread_mutex_unlock(&s_syncOKToSendMutex);
}

void Sender_cancel() {
    pthread_cancel(threadSender);
}

void Sender_waitForShutdown()
{
    pthread_join(threadSender, NULL);

    close(socketDescriptor);

    // Clean up
    free(message);
    message = NULL;
}
