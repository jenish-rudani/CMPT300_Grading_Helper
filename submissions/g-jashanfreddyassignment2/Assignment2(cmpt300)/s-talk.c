#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h> 
#include "list.h"
#include "s-talk.h"

//size 
#define MAX_LEN 1024

// threads
static pthread_t UDPinput_thread;
static pthread_t UDPoutput_thread;
static pthread_t awaitUserInput_thread;
static pthread_t screenOutput_thread;

// condition variables
static pthread_cond_t screenOutput_and_UDPinput_Cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t screenOutput_and_UDPinput_Cond2  = PTHREAD_COND_INITIALIZER;
static pthread_cond_t userinput_and_UDPoutput_Cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t userinput_and_UDPoutput_Cond2= PTHREAD_COND_INITIALIZER;

//mutexes
static pthread_mutex_t screenOutput_and_UDPinput_Mutex= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t userinput_and_UDPoutput_Mutex = PTHREAD_MUTEX_INITIALIZER;

// list initialization
static List *screenOutput_and_UDPinput_shared_list = NULL;
static List *userinput_and_UDPoutput_shared_list = NULL;

// Function implementations

//Creates the list
void initializingLists(void){
    screenOutput_and_UDPinput_shared_list  = List_create();
    userinput_and_UDPoutput_shared_list = List_create();
}

//Creates the threads

void *initializingThreads(NetworkingInfo info){
    pthread_create(&UDPinput_thread, NULL, &receiveThread, &info); // should have return values
    pthread_create(&UDPoutput_thread, NULL, &sendThread, &info);
    pthread_create(&awaitUserInput_thread, NULL, &getinputThread, NULL);
    pthread_create(&screenOutput_thread, NULL, &printOutputThread, NULL);

    pthread_join(UDPinput_thread, NULL);
    pthread_join(UDPoutput_thread, NULL);
    pthread_join(awaitUserInput_thread, NULL);
    pthread_join(screenOutput_thread, NULL);

    return NULL;
} 

//Initializes the socket
struct addrinfo *initializingSocket(void *info){
    NetworkingInfo *hostip;
    hostip = (NetworkingInfo *)info;

    int returnaddr;
    struct addrinfo hints;
    struct addrinfo *results;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    returnaddr = getaddrinfo(hostip->remoteMachinename, hostip->remotePort, &hints, &results);
    if (returnaddr != 0){
        fprintf(stderr, "Error: getadrrInfo function: %s\n", gai_strerror(returnaddr));
        exit(1);
    }
    return results;
}


//Thread to receive the message
void *receiveThread(void *info){
    NetworkingInfo *hostip;
    hostip = (NetworkingInfo *)info;
    int r_socketDescriptor;

    struct sockaddr_in socketaddr;

    r_socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if(r_socketDescriptor == -1){
        perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }

    memset(&socketaddr, 0 , sizeof(socketaddr));
    socketaddr.sin_family = AF_INET;
    socketaddr.sin_port = htons(atoi(hostip->clientPort));
    socketaddr.sin_addr.s_addr = INADDR_ANY;
   

    int rc = bind(r_socketDescriptor, (struct sockaddr *)&socketaddr, sizeof(struct sockaddr_in));
    if(rc == -1 ){
        perror("Failed to bind");
        close(r_socketDescriptor);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in remoteaddr;
    socklen_t remotelen;

    remotelen = sizeof(remoteaddr);

    while (1)
    {
        char receiveItem[MAX_LEN];
        
        int status = recvfrom(r_socketDescriptor, receiveItem, MAX_LEN, 0,(struct sockaddr *)&remoteaddr, &remotelen);
        if ((status) == -1)
        {
            perror("Error in Receiving");
            exit(1);
        }
        
        //ending the program
        if(strncmp("!", receiveItem, strlen(receiveItem) -1) == 0){
            pthread_cancel(UDPinput_thread);
            pthread_cancel(UDPoutput_thread);
            pthread_cancel(awaitUserInput_thread);
            pthread_cancel(screenOutput_thread);
            perror("Good Bye");
            exit(1);
        }

        //critical section
        pthread_mutex_lock(&screenOutput_and_UDPinput_Mutex);
        if (screenOutput_and_UDPinput_shared_list->count == 1) {
            pthread_cond_wait(&screenOutput_and_UDPinput_Cond2,&screenOutput_and_UDPinput_Mutex);
        }
        int ret = List_append(screenOutput_and_UDPinput_shared_list, receiveItem);
       
        pthread_cond_signal(&screenOutput_and_UDPinput_Cond);
        pthread_mutex_unlock(&screenOutput_and_UDPinput_Mutex);
        //end of critical section
    }
    return NULL;
}

//Thread to send the message
void *sendThread(void *info){
    NetworkingInfo *hostip;
    hostip = (NetworkingInfo*)info;

    int status;
    struct addrinfo *pointer;

    for (pointer= hostip->socket_results; pointer!= NULL; pointer= pointer->ai_next) {
        status = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);
        if (status == -1){
            continue;
        }
        break;
    }

    if (pointer == NULL){
        fprintf(stderr, "Error Creating client socket\n");
        exit(2);
    }

    while (1){

        //crtical section
        pthread_mutex_lock(&userinput_and_UDPoutput_Mutex);
        if (userinput_and_UDPoutput_shared_list->count == 0){
            pthread_cond_wait(&userinput_and_UDPoutput_Cond, &userinput_and_UDPoutput_Mutex);
        }

        char *item = List_trim(userinput_and_UDPoutput_shared_list);
        int item_len = strlen(item) + 1; //(because of the \0 character)
        pthread_cond_signal(&userinput_and_UDPoutput_Cond2);
        pthread_mutex_unlock(&userinput_and_UDPoutput_Mutex);
        // end of critical section

        //non critical section
        sendto(status, item, item_len, 0, pointer->ai_addr, pointer->ai_addrlen);
    }
    return NULL;
}

//Thread to receive the input from the screen
void *getinputThread(void *unused){
    while (1)
    {
        char inputItem[MAX_LEN];
        fgets(inputItem, MAX_LEN, stdin);
        
        //ending the program
        if(strncmp("!", inputItem, strlen(inputItem) -1) == 0){
            pthread_cancel(UDPinput_thread);
            pthread_cancel(UDPoutput_thread);
            pthread_cancel(awaitUserInput_thread);
            pthread_cancel(screenOutput_thread);
            perror("Good Bye");
            exit(1);
        }

        //critical section
        pthread_mutex_lock(&userinput_and_UDPoutput_Mutex);
        if (userinput_and_UDPoutput_shared_list->count == 1) {
            pthread_cond_wait(&userinput_and_UDPoutput_Cond2,&userinput_and_UDPoutput_Mutex);
        }
        int ret = List_prepend(userinput_and_UDPoutput_shared_list, inputItem);
        pthread_cond_signal(&userinput_and_UDPoutput_Cond);
        pthread_mutex_unlock(&userinput_and_UDPoutput_Mutex);
        //end of critical section
    }
    return NULL;
}


//for printing messages on screen
void *printOutputThread(){

    while (1)
    {
        //critical section
        pthread_mutex_lock(&screenOutput_and_UDPinput_Mutex);
        if (screenOutput_and_UDPinput_shared_list->count == 0) {
            pthread_cond_wait(&screenOutput_and_UDPinput_Cond, &screenOutput_and_UDPinput_Mutex);
        }
        char *message = List_trim(screenOutput_and_UDPinput_shared_list);
        pthread_cond_signal(&screenOutput_and_UDPinput_Cond2);
        pthread_mutex_unlock(&screenOutput_and_UDPinput_Mutex);
        //end of critical section
        fputs(message, stdout);
     }
    return NULL;
}


