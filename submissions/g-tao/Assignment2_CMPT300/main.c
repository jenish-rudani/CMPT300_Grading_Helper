#include <stdio.h>
#include <stdlib.h> 
#include <netdb.h>
#include <string.h> 
#include <unistd.h>
#include <pthread.h>
#include "list.h"
#include "keyboard.h"
#include "recv.h"
#include "send.h"
#include "screen.h"

#define MSG_MAX_LEN 256


int main(int argc, char **argv) {
    
    int myPort = atoi(argv[1]); 
    int theirPort = atoi(argv[3]);

    printf("s-talk starting at %s with %s\n", argv[1], argv[3]);
    int socketDescriptor;
    //Address
    struct sockaddr_in sin;
    struct sockaddr_in sinRemote;
    memset(&sin, 0, sizeof(sin));
    memset(&sinRemote, 0, sizeof(sinRemote));

    sin.sin_family = AF_INET; // (IPv4)
    sin.sin_addr.s_addr = htonl(INADDR_ANY); //Host to network long
    sin.sin_port = htons(myPort);       //Host to network short
    sinRemote.sin_port = htons(theirPort); ///Host to network short (theirPort)

    //create the socket for UDP
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

    //error handler
    if (socketDescriptor == -1) {
        printf("failed to create socket");
        exit(EXIT_FAILURE);
    }

    //bind the socket to the port (myPort) that we specify
    bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(struct sockaddr_in));

    //startup my modules
    Keyboard_init();
    Send_init(&sinRemote, socketDescriptor);
    Recv_init(&sinRemote, socketDescriptor);
    Screen_init();

    //shutdown my modules
    Send_shutDown();
    Recv_shutDown();
    Keyboard_shutDown();
    Screen_shutDown();
    
   //close socket
    close(socketDescriptor);
    return 0;
}