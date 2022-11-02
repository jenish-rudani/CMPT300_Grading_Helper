#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "input.h"

static pthread_t threadInput;

// Thread for getting input and adding it to the list
void *inputThread(void *arg)
{
    // Initialize shared data
    struct send_data *data = (struct send_data *)arg;

    // Get input and add it to send list which is shared with the sender
    while (1)
    {
        char *input = malloc(MSG_MAX);
        fgets(input, MSG_MAX, stdin);
        input[strlen(input) - 1] = '\0'; // remove '\n'
        pthread_mutex_lock(&data->sendListMutex);
        {
            List_append(data->sendList, input);
            data->flag = 0;
            // Send signal that msg was added
            pthread_cond_signal(&data->okToSendCondVar);
            // Wait for the sender to finish sending first
            while (!data->flag)
            {
                pthread_cond_wait(&data->okToSendCondVar, &data->sendListMutex);
            }
        }
        pthread_mutex_unlock(&data->sendListMutex);
    }
    return NULL;
}

// Create input thread
void Input_init(struct send_data *data)
{
    int rc = pthread_create(&threadInput, NULL, inputThread, (void *)data);
    if (rc)
    {
        printf("Error: %d\n", rc);
    }
}

// Close input thread
void Input_shutdown()
{
    pthread_cancel(threadInput);
}

// Join input thread
void Input_join()
{
    pthread_join(threadInput, NULL);
}