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
#include "sender.h"

int socketId = 0;
struct addrinfo * currentAddress;

void createSendConnection(char * receiverPort, char * receiverName){
    struct addrinfo hints;
    struct addrinfo * remoteAddresses;
    
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int res = getaddrinfo(receiverName, receiverPort, &hints, &remoteAddresses);

    if(res != 0){
        printf("Error: %s\n", gai_strerror(res));
        exit(1);
    }

    for(currentAddress = remoteAddresses; currentAddress !=NULL; currentAddress = currentAddress->ai_next){
        socketId = socket(currentAddress->ai_family, currentAddress->ai_socktype, currentAddress->ai_protocol);
        if(socketId == -1){
            continue;
        }

        break;
    }

    if(currentAddress == NULL){
        printf("Cannot create connection\n");
        exit(1);
    }

    freeaddrinfo(remoteAddresses);

}

void sendMessage(char * message){
    int res = sendto(socketId, message, strlen(message)+1, 0, currentAddress->ai_addr, currentAddress->ai_addrlen);
    if(res == -1){
        printf("Error, Ensure receiver is connected\n");
    }
}

void endSendChatSession(){
    printf("Closing socket...\n");
    close(socketId);
}