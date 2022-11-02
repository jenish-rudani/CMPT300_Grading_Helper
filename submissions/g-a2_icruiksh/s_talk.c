
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> // Header file for sleep

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <curses.h>

#include "s_talk.h"
#include "list.h"



// Defined Parameters

#define IP_ADDR_MAX_LENGTH      INET6_ADDRSTRLEN
#define MACHINE_NAME_MAX_LENGTH 32 // Arbitrary
#define MACHINE_NODE_MAX_LENGTH 64 // "
#define MAX_PORT_NUMBER         65535
#define MIN_PORT_NUMBER         1024
#define MSG_INDENT              24
#define MSG_MAX_LENGTH          128
#define POLLING_WAIT_TIME_S     1
#define PORT_STRING_LENGTH      6 // 5 digits + null character

static const char DISCONNECT_COMMAND[] = "!\0";

static const char LOCAL_MSG_HEADER[] = "<< You >>";

#define USAGE_INSTRUCTIONS "\
    Use ./s-talk [local port number] [remote machine name] [remote port number]\n\
    Ex: ./s-talk 6060 csil-cpu3 6001"



// Static variables

static int localPort = -1;
static int remotePort = -1;
static char remoteMachineName[MACHINE_NAME_MAX_LENGTH];
static char remoteIP[IP_ADDR_MAX_LENGTH];
static char localMachineName[MACHINE_NAME_MAX_LENGTH];

static bool iAmServer = false;
static bool serverStatusIsDecided = false;

static int localSocketFileDesc = -1;
static int remoteSocketFileDesc = -1;

static List *sendQueue = NULL;
static List *printQueue = NULL;
static pthread_mutex_t sendQueueMutex;
static pthread_mutex_t printQueueMutex;

static pthread_t getInputID;
static pthread_t sendMessagesID;
static pthread_t recvMessagesID;
static pthread_t printMessagesID;

static bool startDisconnect = false;



static void getCmdLineArgs(int numArgs, char *cmdLineArgs[])
{
    // General invocation error checking
    if(numArgs != S_TALK_CMD_NUM_ARGS) {
        printf("Error: Wrong number of arguments to s-talk\n%s\n", USAGE_INSTRUCTIONS);
        exit(S_TALK_CMD_BAD_ARGUMENTS);
    }

    localPort = atoi(cmdLineArgs[1]);
    strcpy(remoteMachineName, cmdLineArgs[2]);
    remoteMachineName[MACHINE_NAME_MAX_LENGTH - 1] = '\0';
    remotePort = atoi(cmdLineArgs[3]);
    
    if(localPort > MAX_PORT_NUMBER || localPort < MIN_PORT_NUMBER || remotePort > MAX_PORT_NUMBER || remotePort < MIN_PORT_NUMBER) {
        printf("Error: Port numbers must be within the range [%d, %d]", MIN_PORT_NUMBER, MAX_PORT_NUMBER);
        exit(S_TALK_CMD_BAD_ARGUMENTS);
    }
}

static struct addrinfo* getRemoteAddrInfo(void)
{
    int status;

    char remotePortString[PORT_STRING_LENGTH];
    sprintf(remotePortString, "%d", remotePort);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    // hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    printf("Getting information from %s at port %s...\n", remoteMachineName, remotePortString);

    struct addrinfo *remoteInfo;
    if ((status = getaddrinfo(remoteMachineName, remotePortString, &hints, &remoteInfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    if(remoteInfo->ai_next != NULL) {
        printf("Error: Multiple IP Addresses for this machine name\n");
        exit(1);
    }

    void *addr;
    char *ipver;

    // get the pointer to the address itself,
    // different fields in IPv4 and IPv6:
    if (remoteInfo->ai_family == AF_INET) { // IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)remoteInfo->ai_addr;
        addr = &(ipv4->sin_addr);
        ipver = "IPv4";
    } else { // IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)remoteInfo->ai_addr;
        addr = &(ipv6->sin6_addr);
        ipver = "IPv6";
    }

    // convert the IP to a string and print it:
    char ipstr[IP_ADDR_MAX_LENGTH];
    inet_ntop(remoteInfo->ai_family, addr, ipstr, sizeof ipstr);
    printf("%s %s: %s\n", remoteMachineName, ipver, ipstr);

    return remoteInfo;
}

/**
 * Uses local port and machine name and remote port and machine name to decide which machine should be the server
 * The machine with the smaller port number is the server, unless the ports are the same. Then the ascii values of the machine names are compared
 */
static void decideIfServer(void)
{
    localMachineName[MACHINE_NAME_MAX_LENGTH - 1] = '\0';
    gethostname(localMachineName, MACHINE_NAME_MAX_LENGTH);

    if(localPort == remotePort && 0 == strcmp(localMachineName, remoteMachineName)) {
        printf("Error: Same port on same machine\n");
        exit(1);
    }

    if(localPort < remotePort) {
        iAmServer = true;
        serverStatusIsDecided = true;
    }
    else if(localPort > remotePort) {
        iAmServer = false;
        serverStatusIsDecided = true;
    }
    else {
        for(int i = 0; i < MACHINE_NAME_MAX_LENGTH && !serverStatusIsDecided; i++) {
            if(localMachineName[i] < remoteMachineName[i]) {
                iAmServer = true;
                serverStatusIsDecided = true;
            }
            else if(localMachineName[i] > remoteMachineName[i]) {
                iAmServer = false;
                serverStatusIsDecided = true;
            }
        }
    }

    if(iAmServer) {
        printf("This machine will act as the server\n");
    }
    else {
        printf("The remote machine will act as the server\n");
    }
}

static void connectToRemote(struct addrinfo *remoteAddrInfo)
{
    if(iAmServer) {
        int status;

        char localPortString[PORT_STRING_LENGTH];
        sprintf(localPortString, "%d", localPort);

        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; // Use own IP

        struct addrinfo *localAddrInfo;
        if ((status = getaddrinfo(NULL, localPortString, &hints, &localAddrInfo)) != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
            exit(1);
        }

        localSocketFileDesc = socket(localAddrInfo->ai_family, localAddrInfo->ai_socktype, localAddrInfo->ai_protocol);
        bind(localSocketFileDesc, localAddrInfo->ai_addr, localAddrInfo->ai_addrlen);
        listen(localSocketFileDesc, 1); // Only 1 connection allowed on the incoming queue

        printf("Listening for connection... Press CTRL + C to cancel\n");

        struct sockaddr_storage incomingAddr;
        int addr_size = sizeof(incomingAddr);
        
        remoteSocketFileDesc = accept(localSocketFileDesc, (struct sockaddr *)&incomingAddr, &addr_size);

        if(remoteSocketFileDesc == -1) {
            exit(1);
        }

        printf("Accepted connection from %s\n", remoteMachineName);
    }
    else {
        // remoteAddrInfo already initialized from getaddrinfo() call
        remoteSocketFileDesc = socket(remoteAddrInfo->ai_family, remoteAddrInfo->ai_socktype, remoteAddrInfo->ai_protocol);

        printf("Attempting to connect to %s... Press CTRL + C to cancel\n", remoteMachineName);

        // Poll if connect doesn't work right away
        while(-1 == connect(remoteSocketFileDesc, remoteAddrInfo->ai_addr, remoteAddrInfo->ai_addrlen)) {
            nanosleep((struct timespec[]){{POLLING_WAIT_TIME_S, 0}}, NULL);
        }

        printf("Connected to %s\n", remoteMachineName);
    }
}

static bool isMessageDisconnectCommand(char* msg) {
    if(msg == NULL) {
        return false;
    }
    if(strlen(msg) < 2) {
        return false;
    }

    return msg[0] == '!' && msg[1] == '\n';
}



// Thread Functions

static void *getInput(void *arg)
{
    while(true) {
        // Get message to send from the command line
        char *msg = malloc(MSG_MAX_LENGTH);
        memset(msg, 0, MSG_MAX_LENGTH);
        read(STDIN_FILENO, msg, MSG_MAX_LENGTH);
        msg[MSG_MAX_LENGTH-1] = 0;

        // Overwrite user input with header header and formatted input
        // \033[<N>A moves the cursor up N lines. This is to overwrite the user input with the header + user input
        printf("\033[1A%s", LOCAL_MSG_HEADER);
        for(int i = strlen(LOCAL_MSG_HEADER); i < MSG_INDENT; i++) {
            printf(" ");
        }
        printf("%s", msg);

        // Safely add the message to the send queue
        pthread_mutex_lock(&sendQueueMutex);

            int prependStatus = LIST_FAIL;
            while(prependStatus == LIST_FAIL) {
                prependStatus = List_prepend(sendQueue, msg);
            }
            // Check if the message is the disconnect command (critical section because sendMessages frees msg)
            // This must come after the prepending, so that sendMessages thread doesn't exit
            startDisconnect = isMessageDisconnectCommand(msg);
        
        pthread_mutex_unlock(&sendQueueMutex);

        if(startDisconnect) {
            // Kill recvMessages thread because it's blocking and won't exit automatically when startDisconnect becomes true
            // Other threads are non-blocking
            pthread_cancel(recvMessagesID);
            printf("\nYou are disconnecting\n");
            break;
        }
    }

    return NULL;
}

static void *sendMessages(void *arg)
{    
    while(true) {
        // Wait until there is a message to send
        // while(List_count(sendQueue) == 0 && !startDisconnect);

        // Safely get a message from the send queue
        pthread_mutex_lock(&sendQueueMutex);
            char *msg = (char*)List_trim(sendQueue);
        pthread_mutex_unlock(&sendQueueMutex);

        // Send the message
        if(msg != NULL) {
            int bytesSent = send(remoteSocketFileDesc, msg, strlen(msg), 0);
            free(msg);
        }

        if(startDisconnect && List_count(sendQueue) == 0) {
            break;
        }
    }

    return NULL;
}

static void *recvMessages(void *arg)
{
    while(true) {
        // Receive sent message from socket file descriptor
        char *msg = malloc(MSG_MAX_LENGTH);
        memset(msg, 0, MSG_MAX_LENGTH);
        int recvStatus = recv(remoteSocketFileDesc, msg, MSG_MAX_LENGTH, 0);
        msg[MSG_MAX_LENGTH-1] = 0;

        // Safely add the message to the print queue
        pthread_mutex_lock(&printQueueMutex);

            int prependStatus = LIST_FAIL;
            while(prependStatus == LIST_FAIL) {
                prependStatus = List_prepend(printQueue, msg);
            }
            // Check if received message is the disconnect command (critical section because printMessages frees msg)
            // This must come after the prepending, so that printMessages thread doesn't exit
            startDisconnect = isMessageDisconnectCommand(msg);

        pthread_mutex_unlock(&printQueueMutex);

        if(startDisconnect) {
            // Kill getInput thread because it's blocking and won't exit automatically when startDisconnect becomes true
            // Other threads are non-blocking
            pthread_cancel(getInputID);
            printf("\n%s is disconnecting\n", remoteMachineName);
            break;
        }
    }

    return NULL;
}

static void *printMessages(void *arg)
{
    while(true) {
        // Safely get the message to print from the print queue
        pthread_mutex_lock(&printQueueMutex);
            char *msg = (char*)List_trim(printQueue);
        pthread_mutex_unlock(&printQueueMutex);

        // Print the message
        if(msg != NULL) {
            // Line the messages up to column MSG_INDENT
            printf("<< %s >>", remoteMachineName);
            // strlen(remoteMachineName) + 6 is the length of "<< remoteMachineName >>"
            for(int i = strlen(remoteMachineName) + 6; i < MSG_INDENT; i++) {
                printf(" ");
            }
            printf("%s", msg);

            free(msg);
        }

        if(startDisconnect && List_count(printQueue) == 0) {
            break;
        }
    }

    return NULL;
}



static void closeSockets(void) {
    printf("Closing connection\n");

    if(iAmServer) {
        close(localSocketFileDesc);
    }

    close(remoteSocketFileDesc);
}

static void startTalking(void)
{
    pthread_mutex_init(&sendQueueMutex, NULL);
    pthread_mutex_init(&printQueueMutex, NULL);

    sendQueue = List_create();
    printQueue = List_create();

    if(sendQueue == NULL || printQueue == NULL) {
        printf("Error: Create Buffer Lists Failed\n");
        exit(1);
    }

    printf("Say something!\n");

    pthread_create(&getInputID,         NULL, &getInput, NULL);
    pthread_create(&sendMessagesID,     NULL, &sendMessages, NULL);
    pthread_create(&recvMessagesID,     NULL, &recvMessages, NULL);
    pthread_create(&printMessagesID,    NULL, &printMessages, NULL);

    pthread_join(getInputID, NULL);
    pthread_join(sendMessagesID, NULL);
    pthread_join(recvMessagesID, NULL);
    pthread_join(printMessagesID, NULL);

    // Disconnect sequence
    closeSockets();

    List_free(sendQueue, NULL);
    List_free(printQueue, NULL);

    pthread_mutex_destroy(&sendQueueMutex);
    pthread_mutex_destroy(&printQueueMutex);
}

void s_talk(int numArgs, char *cmdLineArgs[])
{
    getCmdLineArgs(numArgs, cmdLineArgs);

    struct addrinfo *remoteInfo = getRemoteAddrInfo();

    decideIfServer();

    connectToRemote(remoteInfo);
    
    startTalking();
}