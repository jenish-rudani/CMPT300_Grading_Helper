#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "general.h"
#include "receiver.h"
#include "sender.h"
#include "screen.h"
#include "keyboard.h"
#include "list.h"

void run(int, char*, int);

int main(int argc, char** argv){
    // Check number of arguments
    if (argc != 4){
        printf("Wrong number of arguments supplied. Given: %d\n", argc);
        return 0;
    }

    // Get receive port
    char* receiveptr;  
    int receivePort = strtol(argv[1], &receiveptr, 10);
    if(*receiveptr != '\0'){
        printf("Number not supplied for ports\n");
        return 0;
    }

    // Get destination ip
    char userIP[INET_ADDRSTRLEN];
    strcpy(userIP, argv[2]);
    if(userIP == '\0'){
        printf("IP Empty");
        return 0;
    }

    // Get destination port
    char* sendptr;
    int sendPort = strtol(argv[3], &sendptr, 10);
    if(*sendptr != '\0'){
        printf("Number not supplied for ports\n");
        return 0;
    }

    run(receivePort, userIP, sendPort);

    printf("Done \n");

    return 0;
}

// Starts app threads
void run(int receivePort, char* ip, int sendPort){
    // Condition and List variables
    List* receiveList = List_create();
    pthread_cond_t recItemAvailable = PTHREAD_COND_INITIALIZER;
    pthread_cond_t recBufAvailable = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t recOkToPrintMutex = PTHREAD_MUTEX_INITIALIZER;

    int programFinished = 0;        // Boolean for shutting down program
    List* sendList = List_create();
    pthread_cond_t sendItemAvailable = PTHREAD_COND_INITIALIZER;
    pthread_cond_t sendBufAvailable = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t sendOkToPrintMutex = PTHREAD_MUTEX_INITIALIZER;

    // Start modules
    Receiver_init(receiveList, &recItemAvailable, &recBufAvailable, &recOkToPrintMutex, receivePort, &programFinished);
    Screen_init(receiveList, &recItemAvailable, &recBufAvailable, &recOkToPrintMutex);
    Sender_init(sendList, &sendItemAvailable, &sendBufAvailable, &sendOkToPrintMutex, sendPort, ip, &programFinished);
    Keyboard_init(sendList, &sendItemAvailable, &sendBufAvailable, &sendOkToPrintMutex);

    // Program Loop
    while(1){
        if(programFinished == 1){
            printf("Shutdown!..\n");
            break;
        }
    }

    // Shutdown modules
    Sender_shutdown();
    Keyboard_shutdown();
    Screen_shutdown();  
    Receiver_shutdown();

    List_free(sendList, free);
    
    // Mutex/conditon variable deletion
    pthread_mutex_destroy(&sendOkToPrintMutex);
    pthread_cond_destroy(&sendItemAvailable);
    pthread_cond_destroy(&sendBufAvailable);
    pthread_mutex_destroy(&recOkToPrintMutex);
    pthread_cond_destroy(&recItemAvailable);
    pthread_cond_destroy(&recBufAvailable);
}