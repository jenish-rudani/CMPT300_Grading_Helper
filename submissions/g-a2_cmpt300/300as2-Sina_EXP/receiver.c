#include "includes/receiver.h"

#define MAX_LEN 1024

static int status;
static int receiveSocket;
static char *receive_port;
struct addrinfo *recvinfo, hints;
static List *receiverList;
static pthread_t recvThread;
static pthread_t outputThread;
static pthread_mutex_t receivelistMutex = PTHREAD_MUTEX_INITIALIZER;

void Receiver_cancel(void)
{
    pthread_mutex_destroy(&receivelistMutex);
    pthread_cancel(recvThread);
    pthread_cancel(outputThread);
    exit(1);
}

void destroyReceiverMutex()
{
    pthread_mutex_destroy(&receivelistMutex);
}

void initReceiver(char *rport, pthread_t output, pthread_t receive)
{

    receive_port = rport;
    outputThread = output;
    recvThread = receive;
    receiverList = List_create();

    memset(&hints, 0, sizeof hints); // empty the hints struct
    hints.ai_family = AF_INET;       // AF_INET for IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, receive_port, &hints, &recvinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
}

void *printMessage(void *sharedList)
{
    while (1)
    {
        if (List_count(receiverList) > 0)
        {

            char *messagetoprint;
            pthread_mutex_lock(&receivelistMutex);
            {
                messagetoprint = (char *)List_trim(receiverList);
            }
            pthread_mutex_unlock(&receivelistMutex);

            fputs(messagetoprint, stdout);

            fflush(stdout);
        }
    }
    return NULL;
}

void *receiveThread(void *unused)
{
    struct sockaddr_storage mailingInfo;
    socklen_t mailingInfo_size;

    mailingInfo_size = sizeof mailingInfo;

    // Create and bind to socket
    if ((receiveSocket = socket(recvinfo->ai_family,
                                recvinfo->ai_socktype,
                                recvinfo->ai_protocol)) == -1)
    {
        perror("Receiver: Socket");
        exit(1);
    }
    else if (recvinfo == NULL)
    {
        perror("Receiver: Failed to Create");
        exit(1);
    }

    if (bind(receiveSocket,
             recvinfo->ai_addr,
             recvinfo->ai_addrlen) == -1)
    {
        perror("Receive: Bind");
        exit(1);
    }

    while (1)
    {
        /*
         *   READ
         */
        char messageRx[MAX_LEN]; // buffer Rx
        int bytesRx = recvfrom(receiveSocket,
                               messageRx, MAX_LEN - 1, 0,
                               (struct sockaddr *)&mailingInfo, &mailingInfo_size);

        if (bytesRx < 0)
        {
            perror("received bytes<0");
        }

        // Null terminated (string):
        int terminateIdx = (bytesRx < MAX_LEN) ? bytesRx : MAX_LEN - 1;
        messageRx[terminateIdx] = 0;

        if (strcmp(messageRx, "!\n") == 0)
        {
            fputs("exiting s-talk\n", stdout);
            Receiver_cancel();
        }

        pthread_mutex_lock(&receivelistMutex);
        {
            List_prepend(receiverList, (char *)messageRx);
        }
        pthread_mutex_unlock(&receivelistMutex);
    }
    /*
     * Close Socket When Done
     */

    close(receiveSocket); // avoid mem leaks!
}