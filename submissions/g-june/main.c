#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

#include "list.h"
#include "s-talk.h"



int main (int argc, char *argv[]) {
    struct addrinfo hints;

    // Check if there are a correct number of values
    if (argc != 4) {
        printf("Invalid Input Stream");
        exit(1);
    }

    // Initialize hints struct
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    // Initialize s-talk values 
    initializeArguements(argv[1], argv[2], argv[3]);
    
    // Error Checks
    if (createListenerConnection(hints) != 0) {
        printf("CreateLisnterConnection Failed");
        return -1;
    };
    if (createTalkerConnection(hints) != 0) {
        printf("CreateTalkerConnection Failed");
        return -1;
    }
    // Start s-talk
    startCommunication();
    exit(0);
}
