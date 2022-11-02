
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "receiver.h"

int recSocketId = 0;
struct addrinfo * currentAddress;
int LENGTH_OF_MESSAGE = 1000 + 1;
char messageRes [1001];
socklen_t lengthOfAddr;
struct sockaddr_storage remoteAddress;

void createReceiveConnection(char * receiverPort){

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(receiverPort));
    addr.sin_addr.s_addr = INADDR_ANY;
    memset(&addr.sin_zero, '\0', sizeof(addr.sin_zero));

    recSocketId = socket(AF_INET, SOCK_DGRAM, 0);
    if(recSocketId == -1){
            printf("Error, wrong socketId\n");
            exit(1);
        }

    int res = bind(recSocketId, (struct sockaddr *) &addr, sizeof(addr));

    if(res == -1){
        close(recSocketId);
        printf("Error, Bind has an error\n");
        exit(1);
    }

    lengthOfAddr = sizeof(remoteAddress);

}

char * receiveMessage(){
    
    int res = recvfrom(recSocketId, messageRes, LENGTH_OF_MESSAGE-1, 0, (struct sockaddr *) &remoteAddress,  &lengthOfAddr);

    if(res == -1){
        printf("Error while receiving message\n");
        exit(1);
    }

    messageRes[res] = '\0';

    return messageRes;
}

void endRecChatSession(){
    printf("Closing socket...\n");
    close(recSocketId);
}