#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

#include "keyboard.h"
#include "send.h"
#include "shutdown.h"

#define MAX_SIZE 512
static pthread_t thread;

static void* keyboardThread(void* unused) 
{
    char buff[MAX_SIZE];
    while(true) {
        char* result = fgets(buff, MAX_SIZE, stdin);
        // may have hit eof
        if (result == NULL) {
            Shutdown_triggerShutdown();
            return NULL;
        }

        // copy data; sender must free
        int length = strnlen(buff, MAX_SIZE) + 1;
        char *pMessageToQueue = malloc(length);
        strncpy(pMessageToQueue, buff, length);

        Send_sendMessage(pMessageToQueue);
    }
    return NULL;
}

void Keyboard_init()
{
    exitOnFail(
        pthread_create(&thread, NULL, keyboardThread, NULL) == 0,
        "Unable to create keyboard thread");

}
void Keyboard_shutdown()
{
    // pthread_cancel may fail if thread already exited
    pthread_cancel(thread);
    
    exitOnFail(
        pthread_join(thread, NULL) == 0, 
        "Unable to join keyboard thread");
}
