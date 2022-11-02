#include "listHandler.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"

// Condition variables to handle synchronizing
static pthread_cond_t kbdAndSendCondition;
static pthread_cond_t receiveAndPrintCondition;

// Mutexes for adding and removing things from a list
static pthread_mutex_t kbdAndSendMutex;
static pthread_mutex_t receiveAndPrintMutex;

// Class List variables that will be used by the threads
static List* kbdAndSendList;
static List* receiveAndPrintList;

bool initialize_Lists() {
    kbdAndSendList = List_create();
    receiveAndPrintList = List_create();

    // Initialize the condition variables we will be using to handle synchronization as well
    pthread_cond_init(&kbdAndSendCondition, NULL);
    pthread_cond_init(&receiveAndPrintCondition, NULL);

    // Initialize the mutexes we will be using to handle critical section as well
    pthread_mutex_init(&kbdAndSendMutex, NULL);
    pthread_mutex_init(&receiveAndPrintMutex, NULL);

    return (kbdAndSendList != NULL && receiveAndPrintList != NULL);
}

void addToKdbAndSendList(void* itemPtr) {
    int returnVal;
    pthread_mutex_lock(&kbdAndSendMutex);
    {
        returnVal = List_append(kbdAndSendList, itemPtr);
        // printf("%s Just got added to the kdbAndSendList\n", (char*)itemPtr);

        // Signalling that there is an item to remove
        pthread_cond_signal(&kbdAndSendCondition);
    }
    pthread_mutex_unlock(&kbdAndSendMutex);

    if (returnVal != 0) {
        printf("Error when trying to add to kdbAndSendList - Likely list is full\n");
        exit(1);
    }
}

void* removeFromKdbAndSendList() {
    void* returnVal;
    pthread_mutex_lock(&kbdAndSendMutex);
    {
        // Wait until signaled to remove an item (ensuring that there is an item to remove)
        pthread_cond_wait(&kbdAndSendCondition, &kbdAndSendMutex);
        returnVal = List_remove(kbdAndSendList);
        // printf("%s Just got removed from the kdbAndSendList\n", (char*)returnVal);
    }
    pthread_mutex_unlock(&kbdAndSendMutex);

    if (returnVal == NULL) {
        printf("Error when trying to remove from kdbAndSendList - Likely list is empty\n");
        exit(1);
    }

    return returnVal;
}

void addToReceiveAndPrintList(void* itemPtr) {
    int returnVal;
    pthread_mutex_lock(&receiveAndPrintMutex);
    {
        returnVal = List_append(receiveAndPrintList, itemPtr);
        // printf("%s Just got added to the receiveAndPrintList\n", (char*)itemPtr);
        // Signalling that there is an item to remove
        pthread_cond_signal(&receiveAndPrintCondition);
    }
    pthread_mutex_unlock(&receiveAndPrintMutex);

    if (returnVal != 0) {
        printf("Error when trying to add to receiveAndPrintList - Likely list is full\n");
        exit(1);
    }
}

void* removeFromReceiveAndPrintListList() {
    void* returnVal;
    pthread_mutex_lock(&receiveAndPrintMutex);
    {
        // Wait until signaled to remove an item (ensuring that there is an item to remove)
        pthread_cond_wait(&receiveAndPrintCondition, &receiveAndPrintMutex);
        returnVal = List_remove(receiveAndPrintList);
        // printf("%s Just got removed from the receiveAndPrintList\n", (char*)returnVal);
    }
    pthread_mutex_unlock(&receiveAndPrintMutex);

    if (returnVal == NULL) {
        printf("Error when trying to remove from receiveAndPrintList - Likely list is empty\n");
        exit(1);
    }

    return returnVal;
}

void freeListItem(void* item) {
    item = NULL;
    free(item);
}

void cleanupLists() {
    pthread_cond_destroy(&kbdAndSendCondition);
    pthread_cond_destroy(&receiveAndPrintCondition);
    pthread_mutex_destroy(&kbdAndSendMutex);
    pthread_mutex_destroy(&receiveAndPrintMutex);

    List_free(kbdAndSendList, freeListItem);
    List_free(receiveAndPrintList, freeListItem);
}