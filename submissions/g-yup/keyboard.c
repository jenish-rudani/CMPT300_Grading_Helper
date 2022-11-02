#include "list.h"
#include "receiver.h"
#include "keyboard.h"
#include "printer.h"
#include "sender.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MSG_MAX_LEN 100
static List* pList;
static pthread_t threadKeyboard;
static pthread_mutex_t listMutex = PTHREAD_MUTEX_INITIALIZER;
static char* message = NULL;

static void* keyboardThread(void* unused) {
    while (1) {
        // Receive Keyboard input
        fgets(message, MSG_MAX_LEN, stdin);
        // Add message to the List
        Keyboard_addMessageInList(message);
        // Exit if the message is for terminating the program.
        if(!strcmp(message, "!\n")) {
            // Terminate Other Thread : Signal to sender and cancel receiver and printer.
            Sender_signal();
            Receiver_cancel();
            Printer_cancel();
            return NULL;
        }
        message = (char*) malloc(MSG_MAX_LEN*sizeof(char));
        // Signal to the Sender Thread.
        Sender_signal();
    }

    return NULL;
}


void Keyboard_init(){
    message = (char*) malloc(MSG_MAX_LEN*sizeof(char));
    pList = List_create();
    pthread_create(&threadKeyboard, NULL, keyboardThread, NULL);
}

void Keyboard_addMessageInList(char* message) {
    pthread_mutex_lock(&listMutex);
    {
        List_append(pList, message);
    }
    pthread_mutex_unlock(&listMutex);
}

char* Keyboard_pickMessageInList() {
    char* newMessage;
    pthread_mutex_lock(&listMutex);
    {
        newMessage = List_trim(pList);
    }
    pthread_mutex_unlock(&listMutex);
    return newMessage;
}

void Keyboard_cancel() {
    pthread_cancel(threadKeyboard);
}

void Keyboard_waitForShutdown(){ 
    pthread_join(threadKeyboard, NULL);

    // Clean-up.
    // free(message);
    message = NULL;
    List_free(pList, free);
}