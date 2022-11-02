#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "sender.h"
#include "shutdownManager.h"

static pthread_t threadSender;

// Thread for removing message from send list and sending it
void *senderThread(void *arg)
{
    // Initialize shared data
    struct send_data *data = (struct send_data *)arg;

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
        if (connect(socketDescriptor, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("connect");
            close(socketDescriptor);
            continue;
        }
        break;
    }

    // Take message off send list and send it
    while (1)
    {
        pthread_mutex_lock(&data->sendListMutex);
        {
            // Wait for signal that msg was added
            while (data->flag)
            {
                pthread_cond_wait(&data->okToSendCondVar, &data->sendListMutex);
            }
            List_first(data->sendList);
            char *input = List_remove(data->sendList);
            send(socketDescriptor, input, strlen(input), 0);
            // Check if message is closing message
            if (strcmp(input, "!") == 0)
            {
                free(input);
                close(socketDescriptor);
                ShutdownManagerForSender_triggerShutdown();
                return NULL;
            }
            free(input);
            data->flag = 1;
            // Send signal that its done
            pthread_cond_signal(&data->okToSendCondVar);
        }
        pthread_mutex_unlock(&data->sendListMutex);
    }
    close(socketDescriptor);
    return NULL;
}

// Create sender thread
void Sender_init(struct send_data *data)
{
    int rc = pthread_create(&threadSender, NULL, senderThread, (void *)data);
    if (rc)
    {
        printf("Error: %d\n", rc);
    }
}

// Close sender thread
void Sender_shutdown()
{
    pthread_cancel(threadSender);
}

// Join sender thread
void Sender_join()
{
    pthread_join(threadSender, NULL);
}