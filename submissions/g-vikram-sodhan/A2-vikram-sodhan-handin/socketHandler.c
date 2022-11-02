#include "socketHandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keyboardInputThread.h"
#include "listHandler.h"
#include "printScreenThread.h"
#include "receiveUDPThread.h"
#include "sendUDPThread.h"

static int socketDescriptor;            // Return value of socket() for the local port
static struct addrinfo* remoteAddress;  // Remote Machine address
static struct addrinfo* results;

bool isValidPort(int portNumber) {
    return (1024 < portNumber && portNumber < 65535);
}

bool setUpRemoteAddressInfo(char* machineName, char* machinePortNumber) {
    struct addrinfo* results;  // linked list of results that match the given hints
    struct addrinfo hints;     // specifies criteria for selecting the socket address structures returned in remoteMachineSocketStructure

    memset(&hints, 0, sizeof(hints));  // making sure the struct is empty
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;  // UDP stream socket

    if (getaddrinfo(machineName, machinePortNumber, &hints, &results) != 0) {
        printf("ERROR in getAddrInformation from given arguments\n");
        freeaddrinfo(results);  // Result list no longer needed
        return false;
    }

    for (remoteAddress = results; remoteAddress != NULL; remoteAddress = remoteAddress->ai_next) {
        int remoteSocket = socket(remoteAddress->ai_family, remoteAddress->ai_socktype, remoteAddress->ai_protocol);

        if (remoteSocket == -1) {
            continue;
        }
        break;
    }

    if (remoteAddress == NULL) {  // No possible address succeeded
        printf("COULD NOT CONNECT REMOTE MACHINE\n");
        freeaddrinfo(results);  // Result list no longer needed
        return false;
    }
    // Otherwise remoteAdress will contain valid address information to send data too

    return true;
}

int getLocalSocketValue() {
    return socketDescriptor;
}

struct addrinfo* getRemoteMachineAddress() {
    return remoteAddress;
}

int createMyLocalSocket(int portNumber) {
    // Address
    struct sockaddr_in myLocalSocket;

    // Setting up struct
    memset(&myLocalSocket, 0, sizeof(myLocalSocket));
    myLocalSocket.sin_family = AF_INET;
    myLocalSocket.sin_addr.s_addr = htonl(INADDR_ANY);
    myLocalSocket.sin_port = htons(portNumber);

    // Create the socket for UDP
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

    // Bind the socket to the port that we specify
    // bind() Opens the socket
    return (bind(socketDescriptor, (struct sockaddr*)&myLocalSocket, sizeof(myLocalSocket)));
}

void shutDownSocket() {
    keyboardInputThread_shutdown();
    printScreenThread_shutdown();
    receiveUDPThread_shutdown();
    sendUDPThread_shutdown();

    // "close()" didn't work for for some reason.
    // close(socketDescriptor);

    cleanupLists();

    freeaddrinfo(results);  // Result list no longer needed
}
