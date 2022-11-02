#include "includes/sender.h"

#define MAX_LEN 1024

static int status;
static int sendSocket;
static char *remote_cpu;
static char *send_port;
struct addrinfo *sendinfo, hints;
static List *senderList;
static pthread_t senderThread;
static pthread_t inputThread;
static pthread_mutex_t sendlistMutex = PTHREAD_MUTEX_INITIALIZER;

void Sender_cancel(void)
{
    pthread_mutex_destroy(&sendlistMutex);
    pthread_cancel(senderThread);
    pthread_cancel(inputThread);
    exit(1);
}

void destroySenderMutex()
{
    pthread_mutex_destroy(&sendlistMutex);
}

void initSender(char *sport, char *rcpu, pthread_t input, pthread_t sender)
{
    send_port = sport;
    remote_cpu = rcpu;
    senderThread = sender;
    inputThread = input;
    senderList = List_create();

    memset(&hints, 0, sizeof hints); // empty the hints struct
    hints.ai_family = AF_INET;       // AF_INET for IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(remote_cpu, send_port, &hints, &sendinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
}

void *awaitInput(void *sharedList)
{
    char message[MAX_LEN];
    while (1)
    {

        if (fgets(message, sizeof(message), stdin))
        {
            pthread_mutex_lock(&sendlistMutex);
            {
                if (strcmp(message, "\n") != 0)
                {
                    List_prepend(senderList, (char *)message);
                }
            }
            pthread_mutex_unlock(&sendlistMutex);
        }
    }
}

void *sendThread(void *unused)
{
    if ((sendSocket = socket(sendinfo->ai_family,
                             sendinfo->ai_socktype,
                             sendinfo->ai_protocol)) == -1)
    {
        perror("Sender: Socket");
        exit(1);
    }
    else if (sendinfo == NULL)
    {
        perror("Sender: Failed to Create");
        exit(1);
    }

    while (1)
    {
        /*
         * SEND
         */
        if (List_count(senderList) > 0)
        {
            char *messageTx;
            int numBytes;
            pthread_mutex_lock(&sendlistMutex);
            {
                messageTx = (char *)List_trim(senderList);
            }
            pthread_mutex_unlock(&sendlistMutex);

            // Send Response
            numBytes = sendto(sendSocket,
                              messageTx, strlen(messageTx), // message
                              0,
                              sendinfo->ai_addr, sendinfo->ai_addrlen); // where to

            if (strcmp(messageTx, "!\n") == 0)
            {
                fputs("exiting s-talk\n", stdout);
                Sender_cancel();
            }

            if (numBytes == -1)
            {
                printf("numBytes for senders < 0");
                exit(1);
            }
        }
    }
    /*
     * Close Socket When Done
     */

    close(sendSocket);

    return NULL;
}