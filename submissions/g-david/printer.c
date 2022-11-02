#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "printer.h"

static pthread_t threadPrinter;

// Thread for printing messages added to the receive list
void *printerThread(void *arg)
{
    // Initialize shared data
    struct receive_data *data = (struct receive_data *)arg;

    // Take msg off receive list and print it
    while (1)
    {
        pthread_mutex_lock(&data->receiveListMutex);
        {
            // Wait for receiver to add msg to receive list
            while (data->flag)
            {
                pthread_cond_wait(&data->okToReceiveCondVar, &data->receiveListMutex);
            }
            List_first(data->receiveList);
            char *input = List_remove(data->receiveList);
            puts(input);
            free(input);
            data->flag = 1;
            // Signal receiver that print is finished
            pthread_cond_signal(&data->okToReceiveCondVar);
        }
        pthread_mutex_unlock(&data->receiveListMutex);
    }

    return NULL;
}

// Create printer thread
void Printer_init(struct receive_data *data)
{
    int rc = pthread_create(&threadPrinter, NULL, printerThread, (void *)data);
    if (rc)
    {
        printf("Error: %d\n", rc);
    }
}

// Close printer thread
void Printer_shutdown()
{
    pthread_cancel(threadPrinter);
}

// Join printer thread
void Printer_join()
{
    pthread_join(threadPrinter, NULL);
}