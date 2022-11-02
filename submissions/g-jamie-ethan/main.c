/*
 * Ethan Chou
 * 301422045
 * elc10@sfu.ca
 * Jamie Chin
 * 301402387
 * jlc58@sfu.ca
 */

//sections of code are adapted from Beej's guide to socket programming and resources provided
//in assignment description
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>
#include "list.h"
#include "main.h"

#define MAX_BUF 256

static List* messagesToBeSent;
static List* messagesToBePrinted;
pthread_t receiveInputThread, receiveUDPThread, outputThread, sendMessageThread;
char *server_message;
char *userInput;
char *msgOutput;
static int sockfd, recv_sockfd;
static struct addrinfo *servinfo;
static char* portNumString;
static char* remotePortNumber;
static char* remoteMachineName;
static char* newMessage;

//mutex and semaphore/cond'n variables for kb input and sender:
static pthread_mutex_t inputSenderMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t inputSenderCondition = PTHREAD_COND_INITIALIZER;

//mutex and semaphore/cond'n variables for receiver and writer:
static pthread_mutex_t receiveOutputMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t receiveOutputCondition = PTHREAD_COND_INITIALIZER;

void *receiveKeyboardInput(void* joe) {
    while(1) {
        //memory to be malloc'ed for variable addressing purposes
        char* userInputMessage;
        char buffer[MAX_BUF];

        //reset the buffer before reading in each (new) message
        memset(&buffer, 0, MAX_BUF);
        read(0, buffer, MAX_BUF);

        //allocate memory equivalent to the max buffer size and put the buffer msg in there.
        userInputMessage = (char*) malloc(sizeof(char)*(MAX_BUF + 1));
        strcpy(userInputMessage, buffer);

        //add message to back of list to be sent. Since we are modifying data we
        //must use semaphores to satisfy BCC.
        pthread_mutex_lock(&inputSenderMutex);
        List_append(messagesToBeSent, userInputMessage);
        pthread_mutex_unlock(&inputSenderMutex);

        //upon input of the !, the program exits for both.
        if(!strcmp(userInputMessage, "!\n")) {
            //signal the sender to finish what it is doing.
            pthread_mutex_lock(&inputSenderMutex);
            pthread_cond_signal(&inputSenderCondition);
            pthread_mutex_unlock(&inputSenderMutex);

            //end the receive and write thread for the sending port; as we will
            //no longer be receiving any messages b/c of connection termination.
            pthread_cancel(receiveUDPThread);
            pthread_cancel(outputThread);
            return NULL;
        }
        //signal the sender it is good to send the next datagram.
        pthread_mutex_lock(&inputSenderMutex);
        pthread_cond_signal(&inputSenderCondition);
        pthread_mutex_unlock(&inputSenderMutex);
    }
}

void *sendUDPMessage(void* joe) {
    //code adapted from Beej's guide; resource mentioned in Coursys.
    struct addrinfo hints, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, remotePortNumber, &hints, &servinfo) != 0) {
        printf("getaddrinfo error from send msg func.\n");
        exit(-1);
    }

    //for loop from Beej's guide
    for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("listener: socket");
            continue;
        }
        break;
    }

    if (p == NULL) {
        printf("sender socket not created.");
        exit(-1);
    }

    while (1) {
        //mutex/semaphore snippet that waits for the sender condition to be signalled to
        //by the kb input thread to continue.
        pthread_mutex_lock(&inputSenderMutex);
        pthread_cond_wait(&inputSenderCondition, &inputSenderMutex);
        newMessage = List_trim(messagesToBeSent);
        pthread_mutex_unlock(&inputSenderMutex);

        if (newMessage == NULL) {
            printf("list empty; no messages to transmit.\n");
            break;
        }

        if (sendto(sockfd, newMessage, strlen(newMessage), 0, p->ai_addr, p->ai_addrlen) == -1) {
            printf("socket cannot be sent to.\n");
            exit(-1);
        }

        //deallocate the dynamic memory for both sides; if exclamation point is sent
        //end the program.
        if(!strcmp(newMessage,"!\n")) {
            free(newMessage);
            newMessage = NULL;
            return NULL;
        }
        free(newMessage);
        newMessage = NULL;
    }
    return NULL;
}

//code adapted from Beej's guide - section 9.18.0.4
void *receiveUDPMessage(void* joe) {
    struct addrinfo hints, *recv_servinfo, *p;
    char buffer[MAX_BUF];
    char* receivedMessage;
    struct sockaddr_in dest_addr;
    socklen_t dest_addr_len;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, portNumString, &hints, &recv_servinfo) != 0) {
        printf("getaddrinfo error from receive msg func.\n");
        exit(-1);
    }

    //reference: Beej's guide for loop
    for(p = recv_servinfo; p != NULL; p = p->ai_next) {
        recv_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (recv_sockfd == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(recv_sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(recv_sockfd);
            perror("listener: bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        printf("receiver socket not created.");
        exit(-1);
    }

    freeaddrinfo(recv_servinfo);

    while(1) {
        //reset the message buffer whenever a new one is sent
        memset(&buffer, 0, MAX_BUF);
        dest_addr_len = sizeof(dest_addr);
        if (recvfrom(recv_sockfd, buffer, MAX_BUF, 0, (struct sockaddr*) &dest_addr, &dest_addr_len) == -1) {
            perror("receiver: recvfrom error\n");
            exit(-1);
        }

        //malloc memory similar to what occurs in the keyboard input function
        receivedMessage = (char*) malloc(sizeof(char) * (MAX_BUF + 1));
        strcpy(receivedMessage, buffer);

        //add this message (which was received by the sending thread) to the list
        //of messages to be printed. (mutexes cuz list modification again)
        pthread_mutex_lock(&receiveOutputMutex);
        List_append(messagesToBePrinted, receivedMessage);
        pthread_mutex_unlock(&receiveOutputMutex);

        //if user wants to end the program, then let it finish printing (signal
        //to writing thread) to proceed either way. 
        if(!strcmp(receivedMessage, "!\n")) {
            pthread_mutex_lock(&receiveOutputMutex);
            pthread_cond_signal(&receiveOutputCondition);
            pthread_mutex_unlock(&receiveOutputMutex);

            pthread_cancel(receiveInputThread);
            pthread_cancel(sendMessageThread);
            return NULL;
        }
        //signal to writer that it's good to go
        pthread_mutex_lock(&receiveOutputMutex);
        pthread_cond_signal(&receiveOutputCondition);
        pthread_mutex_unlock(&receiveOutputMutex);
    }
}

void *outputToScreen(void* joe) {
    while(1) {
        //mutexes that block this thread until the write condition is woken up or
        //signalled at by the receiving function to proceed printing.
        pthread_mutex_lock(&receiveOutputMutex);
        pthread_cond_wait(&receiveOutputCondition, &receiveOutputMutex);
        newMessage = List_trim(messagesToBePrinted);
        pthread_mutex_unlock(&receiveOutputMutex);

        if(newMessage == NULL) {
            printf("No messages to print!\n");
            return NULL;
        }

        if(puts(newMessage) == -1) {
            perror("\ncan't write to screen\n");
            exit(-1);
        }

        //deallocate the dynamic memory for both sides; if exclamation point is sent
        //end the program.
        if(!strcmp(newMessage, "!\n"))
        {
            free(newMessage);
            newMessage = NULL;
            return NULL;
        }
        free(newMessage);
        newMessage = NULL;
    }
    return NULL;
}

//helper functions for create and join threads + free any memory that may be dangling.
void joinAllThreads() {
    pthread_join(receiveInputThread, NULL);
    pthread_join(sendMessageThread, NULL);
    pthread_join(receiveUDPThread, NULL);
    pthread_join(outputThread, NULL);
}

void createAllThreads() {
    pthread_create(&receiveInputThread, NULL, receiveKeyboardInput, NULL);
    pthread_create(&sendMessageThread, NULL, sendUDPMessage, NULL);
    pthread_create(&receiveUDPThread, NULL, receiveUDPMessage, NULL);
    pthread_create(&outputThread, NULL, outputToScreen, NULL);
}

void shutdownProgram() {
    pthread_mutex_destroy(&inputSenderMutex);
    pthread_mutex_destroy(&receiveOutputMutex);

    pthread_cond_destroy(&inputSenderCondition);
    pthread_cond_destroy(&receiveOutputCondition);

    close(sockfd);
    free(newMessage);
    newMessage = NULL;
    freeaddrinfo(servinfo);
}

int main(int argc, char** argv) {
    //initialization of static variables and important info (like host ports, etc.)
    messagesToBeSent = List_create();
    messagesToBePrinted = List_create();
    portNumString = argv[1];
    remoteMachineName = argv[2];
    remotePortNumber = argv[3];
    if (argc != 4) {
        printf("Usage: ./s-talk <hostport> <remote name> <destport>\n");
        return -1;
    }
    createAllThreads();
    joinAllThreads();
    shutdownProgram();
    return 0;
}