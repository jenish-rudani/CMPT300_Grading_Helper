#include "includes/sockets.h"

// Threads
static pthread_t sendMSGthread;
static pthread_t readMSGthread;
static pthread_t printMSGthread;
static pthread_t IOthread;

// IP Variables
static int status;

// Shared Data and Mutexes

void Sockets_cancel(void)
{
    pthread_cancel(readMSGthread);
    pthread_cancel(sendMSGthread);
    pthread_cancel(IOthread);
    pthread_cancel(printMSGthread);
    exit(1);
}

void Sockets_init(char *rport, char *remoteCPU, char *sport)
{
    initReceiver(rport, printMSGthread, readMSGthread);
    initSender(sport, remoteCPU, sendMSGthread, IOthread);
    pthread_create(&readMSGthread, NULL, &receiveThread, NULL);
    pthread_create(&sendMSGthread, NULL, &sendThread, NULL);
    pthread_create(&IOthread, NULL, &awaitInput, NULL);
    pthread_create(&printMSGthread, NULL, &printMessage, NULL);
}

void Sockets_shutdown(void)
{
    pthread_join(readMSGthread, NULL);
    pthread_join(sendMSGthread, NULL);
    pthread_join(IOthread, NULL);
    pthread_join(printMSGthread, NULL);
    destroyReceiverMutex();
    destroySenderMutex();
}
