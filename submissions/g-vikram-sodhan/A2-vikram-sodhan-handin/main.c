#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keyboardInputThread.h"
#include "listHandler.h"
#include "printScreenThread.h"
#include "receiveUDPThread.h"
#include "sendUDPThread.h"
#include "socketHandler.h"

int main(int argumentCount, char** arguments) {
    // Close program if given any number of arguments that isn't 4
    // example - "s-talk 6060 csil-cpu3 6001"

    if (argumentCount != 4) {
        printf("Invalid number of arguments provided. This program accepts exactly 4 arguments\n");
        return -1;
    }

    // Note that arguments[0] contains the name of the program itself (s-talk)

    int myPortNumber = atoi(arguments[1]);                 // my port number
    char* remoteMachineName = arguments[2];                // remote machine name
    char* remoteMachinePortAsCharacterPtr = arguments[3];  // remote machine port as character pointer. Used for getaddrinfo()
    int remotePortNumber = atoi(arguments[3]);             // remote port number

    if (!isValidPort(remotePortNumber)) {
        printf("Invalid port given as per beejs guide. Port numbers must follow the following relation 1024 < port number < 65535\n");
        return -1;
    }

    printf("myPortNumber - %d\n", myPortNumber);
    printf("myPortNremoteMachineNameumber - %s\n", remoteMachineName);
    printf("remotePortNumber - %d\n", remotePortNumber);

    if (!setUpRemoteAddressInfo(remoteMachineName, remoteMachinePortAsCharacterPtr)) {
        printf("Double check the remote address arguments given.\n");
        return -1;
    }

    if (createMyLocalSocket(myPortNumber) != 0) {
        printf("Error creating local port for receiving data.\n");
        return -1;
    }

    if (!initialize_Lists()) {
        printf("Error when creating lists.\n");
        return -1;
    }

    // initialize and activate the 4 threads needed for the assignment (keyboardInput, printScreen, receiveUDP, sendUDP)
    printf("Activating all threads\n");
    printScreenThread_init();
    receiveUDPThread_init();
    sendUDPThread_init();
    keyboardInputThread_init();

    return 0;
}