#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include <pthread.h>
#include "threads.h"
#include "sender.h"
#include "receiver.h"
#include <stdbool.h>

pthread_mutex_t mutexOut;
pthread_mutex_t spaceAvailableMutex;
pthread_cond_t spaceAvailableCondition;
pthread_cond_t messageAvailableCondition;
pthread_mutex_t messageAvailableMutex;
pthread_mutex_t mutexIn;
pthread_mutex_t spaceAvailableForWritingMutex;
pthread_cond_t spaceAvailableForWritingCondition;
pthread_cond_t messageAvailableForWritingCondition;
pthread_mutex_t messageAvailableForWritingMutex;
char * messageToBeSent;
pthread_t keyboardInputThread, udpOutputThread, udpInputThread, screenOutputThread;

void initThreads(pthread_t thread1, pthread_t thread2, pthread_t thread3, pthread_t thread4){
    keyboardInputThread = thread1;
    udpOutputThread = thread2;
    udpInputThread = thread3;
    screenOutputThread = thread4;
}

void initMutexesAndConditions(){
    pthread_mutex_init(&mutexOut, NULL);
    pthread_mutex_init(&spaceAvailableMutex, NULL);
    pthread_cond_init(&spaceAvailableCondition, NULL);
    pthread_mutex_init(&messageAvailableMutex, NULL);
    pthread_cond_init(&messageAvailableCondition, NULL);

    pthread_mutex_init(&mutexIn, NULL);
    pthread_mutex_init(&spaceAvailableForWritingMutex, NULL);
    pthread_cond_init(&spaceAvailableForWritingCondition, NULL);
    pthread_mutex_init(&messageAvailableForWritingMutex, NULL);
    pthread_cond_init(&messageAvailableForWritingCondition, NULL);

}

void destroyAllMutexesAndConditionVars(){
    pthread_mutex_destroy(&mutexOut);
    pthread_mutex_destroy(&spaceAvailableMutex);
    pthread_cond_destroy(&spaceAvailableCondition);
    pthread_mutex_destroy(&messageAvailableMutex);
    pthread_cond_destroy(&messageAvailableCondition);

    pthread_mutex_destroy(&mutexIn);
    pthread_mutex_destroy(&spaceAvailableForWritingMutex);
    pthread_cond_destroy(&spaceAvailableForWritingCondition);
    pthread_mutex_destroy(&messageAvailableForWritingMutex);
    pthread_cond_destroy(&messageAvailableForWritingCondition);
}

void * getMessage(void * outgoingList){

    List * list = (List *) outgoingList;
    while(1){
        char * message = malloc(sizeof(char) * 500);
        fgets(message, 500, stdin);

        if(List_count(list) >= 100){
            pthread_mutex_lock(&spaceAvailableMutex);
            pthread_cond_wait(&spaceAvailableCondition, &spaceAvailableMutex);
            pthread_mutex_unlock(&spaceAvailableMutex);
        }

        pthread_mutex_lock(&mutexOut);
        List_append(list, (void *) message);
        pthread_mutex_unlock(&mutexOut);
        pthread_mutex_lock(&messageAvailableMutex);
        pthread_cond_signal(&messageAvailableCondition);
        pthread_mutex_unlock(&messageAvailableMutex);
    }
    
    pthread_exit(NULL);
}

void * dispatchMessage(void * outgoingList){

    List * list = (List *) outgoingList;

    while(1){
        
        if(List_count(list) == 0){
            pthread_mutex_lock(&messageAvailableMutex);
            pthread_cond_wait(&messageAvailableCondition, &messageAvailableMutex);
            pthread_mutex_unlock(&messageAvailableMutex);
        }

        pthread_mutex_lock(&mutexOut);
        List_first(list);

        messageToBeSent = (char *)List_remove(list);

        pthread_mutex_unlock(&mutexOut);

        pthread_mutex_lock(&spaceAvailableMutex);
        pthread_cond_signal(&spaceAvailableCondition);
        pthread_mutex_unlock(&spaceAvailableMutex);
        
        sendMessage(messageToBeSent);

        if(strcmp(messageToBeSent, "!\n") == 0){
            cancelAllThreads();
        }
    }

   pthread_exit(NULL);

}

void * getMessageFromNetwork(void * incomingList){
    List * list = (List *) incomingList;

    while(1){
            char * message = receiveMessage();
            char * messageReceived = (char *)malloc(sizeof(char) * (strlen(message)+1));
            strcpy(messageReceived, message);

            if(List_count(list) >= 100){
                pthread_mutex_lock(&spaceAvailableForWritingMutex);
                pthread_cond_wait(&spaceAvailableForWritingCondition, &spaceAvailableForWritingMutex);
                pthread_mutex_unlock(&spaceAvailableForWritingMutex);
            }

            pthread_mutex_lock(&mutexIn);
            List_append(list, (void *) messageReceived);
            pthread_mutex_unlock(&mutexIn);
            pthread_mutex_lock(&messageAvailableForWritingMutex);
            pthread_cond_signal(&messageAvailableForWritingCondition);
            pthread_mutex_unlock(&messageAvailableForWritingMutex);

    }

    pthread_exit(NULL);
}

void * printMessageOnScreen(void * incomingList){
    List * list = (List *) incomingList;

    while(1){
        if(List_count(list) == 0){

            pthread_mutex_lock(&messageAvailableForWritingMutex);
            pthread_cond_wait(&messageAvailableForWritingCondition, &messageAvailableForWritingMutex);
            pthread_mutex_unlock(&messageAvailableForWritingMutex);
        }

        pthread_mutex_lock(&mutexIn);
        List_first(list);
        char * messageToBeWritten = (char *) List_remove(list);
        pthread_mutex_unlock(&mutexIn);

        pthread_mutex_lock(&spaceAvailableForWritingMutex);
        pthread_cond_signal(&spaceAvailableForWritingCondition);
        pthread_mutex_unlock(&spaceAvailableForWritingMutex);

        fputs(messageToBeWritten, stdout);

        if(strcmp(messageToBeWritten, "!\n") == 0){
            cancelAllThreads();
        }
    }

    pthread_exit(NULL);
}

void cancelAllThreads(){
    pthread_cancel(keyboardInputThread);
    pthread_cancel(udpInputThread);
    pthread_cancel(udpOutputThread);
    pthread_cancel(screenOutputThread);
}