#include "includes/userIO.h"

void* keyboardRead(void *arg){
    userIO_IN *info = (userIO_IN *) arg;
    List *keyboardList = info->data;
    pthread_mutex_t *kbMutex = info->mutex; // Mutex from the struct(shared mutex)
    char read[MAXREADSIZE];
    char *charToAdd;
    while(1){
        while(fgets(read, MAXREADSIZE, stdin) == NULL); // Busy-waits until there is a result from keyboard
        if (!strcmp(read, "!\n")){ // If from keyboard ! then sets endFlag which prompts the code to exit
            *(info->endFlag) = true;
        }
        if(strcmp(read, "\n") != 0){ // If just an endLine just ignore, else send results
            int i = 0;
            pthread_mutex_lock(kbMutex);
            while(read[i] != '\0'){ // fgets puts an nullbyte at the end, so write until reaching nullbyte
                if (List_count(keyboardList) >= LIST_MAX_NUM_NODES){ // If list is full then give up control
                    pthread_mutex_unlock(kbMutex);
                    while(List_count(keyboardList) >= LIST_MAX_NUM_NODES); // Busy-waits until list is not full
                    pthread_mutex_lock(kbMutex); // Locks so we can write with no problems
                }
                charToAdd = (char *)malloc(sizeof(char));
                *charToAdd = read[i++]; // Sending through list char by char
                List_prepend(keyboardList, charToAdd); // Adds to the front, so list becomes First in First out
            }
            pthread_mutex_unlock(kbMutex);
        }
        read[0] = '\0';
    }
    return NULL;
}

void* screenWrite(void *arg){
    userIO_IN *info = (userIO_IN *) arg;
    List *screenList = info->data;
    pthread_mutex_t *sMutex = info->mutex; // Mutex from the struct(shared mutex)
    char *charToWrite;
    while(1){
        while(List_count(screenList) <= 0); // Busy-waits until there is something in the list
        pthread_mutex_lock(sMutex);
        while(List_count(screenList) > 0){ // Keeps on writing until list is empty
            charToWrite = (char *)List_trim(screenList); // Trims gets the last element of list
            fputc(*charToWrite, stdout); // Prints to screen char by char as list is char by char
            free(charToWrite); // Make sure to free since List_trim mallocs
        }
        pthread_mutex_unlock(sMutex);
    }
    return NULL;
}