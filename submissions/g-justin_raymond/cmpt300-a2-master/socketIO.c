#include "includes/socketIO.h"

void* UDP_recv(void *args){
    // Extracting needed data from input struct
    socketIN *info = (socketIN *) args;
    List *screenList = info->data;
    int *rcvSocket = info->socketfd;
    char *myPortNumber = *(info->myPort);
    bool *endFlag = info->endFlag;
    pthread_mutex_t *screenListMutex = info->mutex;

    //Initializations
    char msg[MAXMSGSIZE];
    char ip4[INET_ADDRSTRLEN];
    int msgSize, rv;
    struct sockaddr_storage remoteAddr;
    socklen_t remoteAddr_len;
    remoteAddr_len = sizeof remoteAddr; 
    struct addrinfo hints, *res, *p;
    char *charToAdd;

    // Zero the struct for hints, initialize essential attributes
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    // Acquire address information given a port number, and desirables (hints),
    // put address information into res
    if ((rv = getaddrinfo(NULL, myPortNumber, &hints, &res)) != 0) {
        fprintf(stderr, "Error with: getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }

    for (p = res; p != NULL; p = p->ai_next){
        // Receive socket file descriptor with the respective address information
        *rcvSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (*rcvSocket == -1){
            perror("Receiver: Socket\n");
            continue;
        }

        if (bind(*rcvSocket, p->ai_addr, p->ai_addrlen) == -1){
            close(*rcvSocket);
            perror("Receiver: Bind\n");
            continue;
        }
        break;
    }

    // Error checking 
    if (p == NULL){
        printf("Receiver: Failed to bind\n");
    }

    // Free address info, not needed anymore
    freeaddrinfo(res);

    // Always loop here, waiting for an output from the socket
    while(1){
        while ((msgSize = recvfrom(*rcvSocket, msg, MAXMSGSIZE-1, 0, (struct sockaddr *)&remoteAddr, &remoteAddr_len)) == -1);

        if (msg[0] == '!' && msg[1] == '\n'){ // "!\n" exit case
            *endFlag = true;
        }
        
        msg[msgSize] = '\0'; // Ensure message ends with null character

        pthread_mutex_lock(screenListMutex);
        for (int i = 0; i < msgSize; i++){
            if(List_count(screenList) >= LIST_MAX_NUM_NODES){   // Do not add to list if full
                pthread_mutex_unlock(screenListMutex);
                while(List_count(screenList) >= LIST_MAX_NUM_NODES);
                pthread_mutex_lock(screenListMutex);
            }
            charToAdd = (char *)malloc(sizeof(char));
            *charToAdd = msg[i];
            List_prepend(screenList, charToAdd);
        }
        pthread_mutex_unlock(screenListMutex);

    }

    return NULL;
}

void* UDP_send(void *args){
    // Extracting needed data from input struct
    socketIN *info = (socketIN *) args;
    List *keyboardList = info->data;
    int *sendSocket = info->socketfd;
    char *remotePortNumber = *(info->remotePort);
    char *remoteMachineName = *(info->remoteMachineName);
    pthread_mutex_t *keyboardListMutex = info->mutex;

    // Initializations
    int rv;
    struct sockaddr_in remote_addr;
    char ip4[INET_ADDRSTRLEN];
    char msg[MAXMSGSIZE];
    char *aChar;
    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // Acquire address information given a port number, and desirables (hints),
    // and also the hostname. Put address information into res
    if ((rv = getaddrinfo(remoteMachineName, remotePortNumber, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }

    for (p = res; p != NULL; p = p->ai_next){
        // Acquire socket file descriptor with the respective address information
        *sendSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	    if (*sendSocket == -1){
            close(*sendSocket);
            perror("Sender: Socket");
            continue;
        }
        break;
    }

    // Error checking
    if (p == NULL){
        printf("Sender: failed to create socket\n");
    }

    printf("Starting S-talk session to %s\n", remoteMachineName);

    // Always loop here, waiting for a character coming from the keyboardList
    while(1){
        while(List_count(keyboardList) <= 0);

        int msgSize = 0;
        char toAdd;

        pthread_mutex_lock(keyboardListMutex);
        while(List_count(keyboardList) > 0){
            aChar = (char *)List_trim(keyboardList);
            msg[msgSize++] = *aChar;
            free(aChar);
        }
        pthread_mutex_unlock(keyboardListMutex);

        msg[msgSize] = '\0';    // Ensure message ends with null character
        int len = sendto(*sendSocket, msg, msgSize, 0, p->ai_addr, p->ai_addrlen);
        if (len == -1){
            perror("Sender: sendto");
            exit(1);
        }
        memset(msg, 0, msgSize);    // Clear the message string
    }

    return NULL;
}