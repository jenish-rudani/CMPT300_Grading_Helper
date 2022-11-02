#include "sender.h"
#include "keyboard.h"
#include "printer.h"
#include "list.h"
#include "receiver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>

// Synchronization
static pthread_cond_t s_syncOKToPrintCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncOKToPrintMutex = PTHREAD_MUTEX_INITIALIZER;

#define MSG_MAX_LEN 100
static pthread_t threadPrinter;
static char *message;

static void *printerThread(void* unused) {
    while (1) {
        pthread_mutex_lock(&s_syncOKToPrintMutex);
        {
            pthread_cond_wait(&s_syncOKToPrintCondVar, &s_syncOKToPrintMutex);
        }
        pthread_mutex_unlock(&s_syncOKToPrintMutex);

        message = Receiver_pickMessageInList();

        write(1, message, strlen(message));

        if(!strcmp(message, "!\n")) {
            return NULL;
        }
    }
    return NULL;
}


void Printer_init() {
    pthread_create(&threadPrinter, NULL, printerThread, NULL);
}

void Printer_signal() {
    pthread_mutex_lock(&s_syncOKToPrintMutex);
    {
        pthread_cond_signal(&s_syncOKToPrintCondVar);
    }
    pthread_mutex_unlock(&s_syncOKToPrintMutex);
}

void Printer_cancel() {
    pthread_cancel(threadPrinter);
}

void Printer_waitForShutdown() {
    pthread_join(threadPrinter, NULL);
    // Clean up
    free(message);
    message = NULL;
}