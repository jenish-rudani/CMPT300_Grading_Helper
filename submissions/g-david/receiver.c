#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "receiver.h"
#include "shutdownManager.h"

static pthread_t threadReceiver;

// Thread for receiving message and adding it to the receive list
void *receiverThread(void *arg)
{
    // Initialize shared data
    struct receive_data *data = (struct receive_data *)arg;

    // Create socket
    struct addrinfo sin, *serverInfo, *p;
    memset(&sin, 0, sizeof(sin));
    sin.ai_family = AF_INET;
    sin.ai_socktype = SOCK_DGRAM;
    getaddrinfo(data->ip, data->port, &sin, &serverInfo);
    int socketDescriptor;
    for (p = serverInfo; p != NULL; p = p->ai_next)
    {
        if ((socketDescriptor = socket(p->ai_family, p->ai_socktype,
                                       p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }
        if (bind(socketDescriptor, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("connect");
            close(socketDescriptor);
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        // looped off the end of the list with no connection
        fprintf(stderr, "failed to connect\n");
        exit(2);
    }

    char *tmsg;
    // Add received message to receive list and signal printer to print
    while (1)
    {
        tmsg = malloc(MSG_MAX);
        pthread_mutex_lock(&data->receiveListMutex);
        {
            int size = recv(socketDescriptor, tmsg, MSG_MAX, 0);
            char *msg = malloc(size);
            msg = tmsg;
            // Check if message is closing message
            if (strcmp(msg, "!") == 0)
            {
                free(msg);
                close(socketDescriptor);
                ShutdownManagerForReceiver_triggerShutdown();
                return NULL;
            }
            List_append(data->receiveList, msg);
            data->flag = 0;
            // Signal printer to print
            pthread_cond_signal(&data->okToReceiveCondVar);
            // Wait for printer to finish printing
            while (!data->flag)
            {
                pthread_cond_wait(&data->okToReceiveCondVar, &data->receiveListMutex);
            }
        }
        pthread_mutex_unlock(&data->receiveListMutex);
    }
    close(socketDescriptor);
    return NULL;
}

// Create receiver thread
void Receiver_init(struct receive_data *data)
{
    int rc = pthread_create(&threadReceiver, NULL, receiverThread, (void *)data);
    if (rc)
    {
        printf("Error: %d\n", rc);
    }
}

// Close receiver thread
void Receiver_shutdown()
{
    pthread_cancel(threadReceiver);
}

// Join receiver thread
void Receiver_join()
{
    pthread_join(threadReceiver, NULL);
}