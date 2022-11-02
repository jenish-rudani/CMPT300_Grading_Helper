#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

#include "list.h"
#include "s-talk.h"

// Buffers
static char receiveBuffer[MAX_DATA_SIZE];
static char writeBuffer[MAX_DATA_SIZE];
static char *sendBuffer, *readBuffer;
// Sockets
static struct addrinfo *remoteInfo, *hostInfo, *p;
static struct sockaddr_storage remoteAddr;
static int addrCheck, listnerSocket, talkerSocket, numBytes;
static char *hostPort, *remotePort, *remoteName;
static socklen_t addrLen;
// Lists
static List *receiveArray, *sendArray;
// Threads
static pthread_t threadInput, threadReceive, threadOutput, threadSend;
// Semaphores
static pthread_mutex_t messageRecievedLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t messageSentLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t recieveArrayLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t sendArrayLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t stdoutLock= PTHREAD_MUTEX_INITIALIZER;
// Conditions
static pthread_cond_t messageRecievedCond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t messageSentCond = PTHREAD_COND_INITIALIZER;
int connectionRunning = 1;
int messageRecieved = 0;
int messageSent = 0;


// Take the arguments passed through main and initialize its corresponding variables.
void initializeArguements(char* argHPort, char* argRName, char* argRPort) {
    // Initialize arguments after taking them from main
    hostPort = argHPort;
    remoteName = argRName;
    remotePort = argRPort;

    // Initialize buffer storage
    receiveArray = List_create();
    sendArray = List_create();
    List_append(sendArray, "blank");
    List_append(receiveArray, "blank");
}

// Initialize the connection by calling checkConnection.
// Initialize the threads for all the processes.
void startCommunication() {

    //Check that both clients are connected to one another
    int connectionCheck = checkConnection();
    if (connectionCheck == 1) {
        printf("Connection Successfully Established.\n");
    } else { 
        exit(0);
    }
    printf("You can now talk to %s. Try and Send a message!\n\n", remoteName);
    // Initialize the threads
    pthread_create( &threadInput, NULL, receiveInput, NULL);
    pthread_create( &threadReceive, NULL, receiveData, NULL);
    pthread_create( &threadOutput, NULL, sendOutput, NULL);
    pthread_create( &threadSend, NULL, sendData, NULL);
    // Upon thread exit, close all the sockets and close sockets
    pthread_join(threadReceive, NULL);
    pthread_exit(NULL);
    close(listnerSocket);
    close(talkerSocket);
    exit(0);
}

// The UDP input thread, on receipt of input from the remote s-talk client, will put the 
// message onto the list of messages that need to be printed to the local screen.
void* receiveData(void* unused) {

    // Keep running until stopped
    while (connectionRunning == 1) {
        // Wait for a packet to be receieved
        if ((numBytes = recvfrom(listnerSocket, receiveBuffer, MAX_DATA_SIZE-1, 0, (struct sockaddr*)&remoteAddr, &addrLen)) == -1) {
            pthread_mutex_lock(&stdoutLock);
                printf("Receiving Data Failed");
            pthread_mutex_unlock(&stdoutLock);
            exit(1);
        }
        receiveBuffer[numBytes] = '\0';
        // If the packet is an '!', close connection
        if (receiveBuffer[0] == '!') {
            closeConnection();
            return NULL;
        }

        // Lock access to the buffer array;
        pthread_mutex_lock(&recieveArrayLock);
            List_insert_after(receiveArray, &receiveBuffer);
            List_first(receiveArray);
        pthread_mutex_unlock(&recieveArrayLock);

        // Lock access to messageRecieved
        pthread_mutex_lock(&messageRecievedLock);
            // Signal readInput that a message has been recieved
            messageRecieved = 1;
            pthread_cond_signal(&messageRecievedCond);
        pthread_mutex_unlock(&messageRecievedLock);    
    }
    return NULL;
}

// The screen output thread will take each message off this list and output it to the screen.
void* receiveInput(void* unused) {

    // Keep running until stopped
    while (connectionRunning == 1) {
        pthread_mutex_lock(&messageRecievedLock);
            while (messageRecieved == 0) {
                pthread_cond_wait(&messageRecievedCond, &messageRecievedLock);
            }   
            // Reset it back to 0 after letting it through
            messageRecieved = 0;  
        pthread_mutex_unlock(&messageRecievedLock);   

        // Lock access to the buffer array;
        pthread_mutex_lock(&recieveArrayLock);
            readBuffer = (char*)List_trim(receiveArray);
        pthread_mutex_unlock(&recieveArrayLock);
        
        // Lock access to the stdout;
        pthread_mutex_lock(&stdoutLock);
            printf("\rFrom %s -> %s\n", remoteName, readBuffer);
        pthread_mutex_unlock(&stdoutLock);
    }
    return NULL;
}


// The keyboard input thread, on receipt of input, adds the input to the list of messages 
// that need to be sent to the remote s-talk client. 
void* sendOutput(void* unused) {

    // Keep running until stopped
    while (connectionRunning == 1) {
        // Method to remove \n from fgets
        writeBuffer[strcspn(writeBuffer, "\n")] = 0;
        // Lock access to stdout and obtain user input afterwards
        pthread_mutex_lock(&stdoutLock);
            printf("To %s -> ", remoteName);
        pthread_mutex_unlock(&stdoutLock);
        fgets(writeBuffer, 100, stdin);

        // Lock access to sendArray
        pthread_mutex_lock(&sendArrayLock);
            List_insert_after(sendArray, &writeBuffer);
            List_first(sendArray);
        pthread_mutex_unlock(&sendArrayLock);  

        // Lock access to messageSent
        pthread_mutex_lock(&messageSentLock);
            // Signal readInput that a message has been recieved
            messageSent = 1;
            pthread_cond_signal(&messageSentCond);
        pthread_mutex_unlock(&messageSentLock);       
    }
    return NULL;
}

// The UDP output thread will take each message off this list and send it over the network 
// to the remote client.
void* sendData(void* unused) {

    // Keep running until stopped
    while (connectionRunning == 1) {
        pthread_mutex_lock(&messageSentLock);
            while (messageSent == 0) {
                // Wait until a message to be sent has been stored in the list
                pthread_cond_wait(&messageSentCond, &messageSentLock);
            }   
            // Reset it back to 0 after letting it through
            messageSent = 0;  
        pthread_mutex_unlock(&messageSentLock);   

        // Lock access to the buffer array
        // Retrieve the string stored in the list
        pthread_mutex_lock(&sendArrayLock);
            sendBuffer = (char*)List_trim(sendArray);
        pthread_mutex_unlock(&sendArrayLock);
        
        // Send the string over the internet
        if (sendto(talkerSocket, sendBuffer, strlen(sendBuffer), 0, p->ai_addr, p->ai_addrlen) == -1) {
            pthread_mutex_lock(&stdoutLock);
                printf("Sending Data Failed");
            pthread_mutex_unlock(&stdoutLock);
            exit(1);
        }
    }
    return NULL;
}

// Send out an initial exchange of packets with the specificed remote port.
// If communication (send and recieve) is successful, return 1.
int checkConnection() {
    char testBuffer[10];
    char* testOutput = "First";
    addrLen = sizeof remoteAddr;
    printf("\nWaiting for Response From %s - Port: %s ...\n", remoteName, remotePort);

    // Send a packet into outer space, regardless of if it is received or not
    if (sendto(talkerSocket, testOutput, strlen(testOutput), 0, p->ai_addr, p->ai_addrlen) == -1) {
        printf("Package not recieved");
        exit(1);
    }
    printf("Waiting to be Contacted ...\n\n");

    // Wait for a message to come 
    if ((numBytes = recvfrom(listnerSocket, testBuffer, 9, 0, (struct sockaddr*)&remoteAddr, &addrLen)) == -1) {
        perror("recvfrom");
        printf("Receiving Data Failed");
        exit(1);
    }
    testBuffer[numBytes] = '\0';

    // If the message obtained is "First", signifiying that it is the first process to start,
    // update message to "Second" to prevent the second client to not send the extra message
    if (strcmp(testBuffer,"First") == 0) {
        testOutput = "Second";
        if (sendto(talkerSocket, testOutput, strlen(testOutput), 0, p->ai_addr, p->ai_addrlen) == -1) {
            printf("Package not recieved");
            exit(1);
        }
    }
    return 1;
}

// When an "!" is received, this function gets called and sends it back to the other client.
// After, connectionRunning is set to 0, ending the while loop of all threads. 
void closeConnection() {
    pthread_mutex_lock(&stdoutLock);
        printf("\nConnection Closed with %s - Port: %s\n\n", remoteName, remotePort);
    pthread_mutex_unlock(&stdoutLock);
    
    // Send over an "!" to end the other client as well
    if (sendto(talkerSocket, "!", 1, 0, p->ai_addr, p->ai_addrlen) == -1) {
        pthread_mutex_lock(&stdoutLock);
            printf("Sending Data Failed");
        pthread_mutex_unlock(&stdoutLock);
        exit(1);
    }
    connectionRunning = 0;
}

// Initialize a socket and bind it to the specified internet address.
int createListenerConnection(struct addrinfo hints) {
    // Initialize host address information
    if ((addrCheck = getaddrinfo(NULL, hostPort, &hints, &hostInfo)) != 0) {
        printf("Failed to get host address info");
        return 1;
    }

    // Loop through all the results and connect to the first viable connection
    for (p = hostInfo; p != NULL; p = p->ai_next) {
        if ((listnerSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("listener: socket");
            // Socket connection returns an error; skip iteration
            continue;
        }
        if (bind(listnerSocket, p->ai_addr, p->ai_addrlen) == -1) {
            // Binding to socket returns an error; skip iteration
            close(listnerSocket);
            perror("listener: bind");
            continue;
        } 
        // Connection is established; Break out of loop
        break;
    }
    // If the list has been traversed fully till NULL, connection failed
    if (p == NULL) {
        printf("Failed to bind socket");
        return 2;
    }
    return 0;
}

// Initialize a socket to sent data through. 
int createTalkerConnection(struct addrinfo hints) {
    // Initialize host address information
    if ((addrCheck = getaddrinfo(NULL, remotePort, &hints, &remoteInfo)) != 0) {
        printf("Failed to get remote address info");
        return 1;
    }

    // Loop through all the results and make a socket without binding
    for (p = remoteInfo; p != NULL; p = p->ai_next) {
        if ((talkerSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("listener: socket");
            // Socket connection returns an error; skip iteration
            continue;
        }
        // Connection is established; Break out of loop
        break;
    }
    // If the list has been traversed fully till NULL, socket creation failed
    if (p == NULL) {
        printf("Failed to bind socket");
        return 2;
    }
    return 0;
}