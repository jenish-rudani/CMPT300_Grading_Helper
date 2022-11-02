#include <stdio.h>
#include "list.h"
#include "sender.h"
#include "receiver.h"
#include "interpreter.h"
#include <pthread.h>
#include "threads.h"

void freeListNode(void * pItem){
    free(pItem);
    printf("List freed\n");
}

int main(void){
    List * incomingList;
    List * outgoingList;
    const RECEIVE_MESSAGE_LENGTH = 500;

    pthread_t keyboardInputThread, udpOutputThread, udpInputThread, screenOutputThread;
    
    initMutexesAndConditions();

    int COMMAND_LENGTH = 100;
    char command [COMMAND_LENGTH];
    char * machineInfo[3];

    readCommand(command, COMMAND_LENGTH);
    bool isValid = validateCommand(command);

    if(isValid){
        splitCommand(machineInfo, command);
        createSendConnection(machineInfo[2], machineInfo[1]);
        createReceiveConnection(machineInfo[0]);

        incomingList = List_create();
        outgoingList = List_create();

        int res = pthread_create(&keyboardInputThread, NULL, getMessage, (void *) outgoingList);
        if(res != 0){
            printf("Could not create keyboard thread\n");
            exit(1);
        }

        res = pthread_create(&udpOutputThread, NULL, dispatchMessage, (void *) outgoingList);
        if(res != 0){
            printf("Could not create udp output thread\n");
            exit(1);
        }

        res = pthread_create(&udpInputThread, NULL, getMessageFromNetwork, (void *) incomingList);
        if(res != 0){
            printf("Could not create udp input thread\n");
            exit(1);
        }

        res = pthread_create(&screenOutputThread, NULL, printMessageOnScreen, (void *) incomingList);
        if(res != 0){
            printf("Could not create screen output thread\n");
            exit(1);
        }

        initThreads(keyboardInputThread, udpOutputThread, udpInputThread, screenOutputThread);

        pthread_join(keyboardInputThread, NULL);
        pthread_join(udpOutputThread, NULL);
        pthread_join(udpInputThread, NULL);
        pthread_join(screenOutputThread, NULL);

        List_free(incomingList, &freeListNode);
        List_free(outgoingList, &freeListNode);
        destroyAllMutexesAndConditionVars();

        endSendChatSession();
        endRecChatSession();
        

        printf("Chat session ...... ended\n");

    }
    else{
        printf("Invalid\n\n");
    }

   

    return 0;
}