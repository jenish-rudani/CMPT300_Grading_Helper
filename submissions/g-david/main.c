#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "list.h"
#include "data.h"
#include "input.h"
#include "sender.h"
#include "receiver.h"
#include "printer.h"

// Shared data
struct send_data *sdata;
struct receive_data *rdata;

int main(int argc, char *args[])
{
    // Initialize data structs
    sdata = malloc(sizeof(struct send_data));
    rdata = malloc(sizeof(struct receive_data));

    // Initialize ip
    sdata->ip = args[2];
    rdata->ip = args[2];

    // Initializing ports
    char *myPort, *theirPort;
    // myPort = strtol(args[1], NULL, 10);
    myPort = args[1];
    theirPort = args[3];
    // theirPort = strtol(args[3], NULL, 10);

    // Create struct with all data for input and sending
    sdata->sendList = List_create();
    sdata->port = theirPort;
    pthread_mutex_t sendListMutex = PTHREAD_MUTEX_INITIALIZER;
    sdata->sendListMutex = sendListMutex;
    pthread_cond_t okToSendCondVar = PTHREAD_COND_INITIALIZER;
    sdata->okToSendCondVar = okToSendCondVar;
    sdata->flag = 1;

    // Create struct with all data for receiving and printing
    rdata->receiveList = List_create();
    rdata->port = myPort;
    pthread_mutex_t receiveListMutex = PTHREAD_MUTEX_INITIALIZER;
    rdata->receiveListMutex = receiveListMutex;
    pthread_cond_t okToReceiveCondVar = PTHREAD_COND_INITIALIZER;
    rdata->okToReceiveCondVar = okToReceiveCondVar;
    rdata->flag = 1;

    // Create threads
    Input_init(sdata);
    Sender_init(sdata);
    Printer_init(rdata);
    Receiver_init(rdata);

    printf("S_TALK STARTED\n");

    // Wait till Sender thread finishes (sender checks input value and returns NULL)
    Sender_join();

    // Cleanup
    pthread_mutex_destroy(&receiveListMutex);
    pthread_mutex_destroy(&sendListMutex);
    pthread_cond_destroy(&okToReceiveCondVar);
    pthread_cond_destroy(&okToSendCondVar);

    // Done
    printf("S-TALK ENDED\n");

    return 0;
}