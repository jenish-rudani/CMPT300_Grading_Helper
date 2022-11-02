#include <string.h>
#include <unistd.h>
#include "socket.h"

// initializing the socket
int Socket_init(int myPort, int connectPort, struct sockaddr_in* s, struct sockaddr_in* sinRemote) {
    // clearing out garbage values
    memset(s, 0, sizeof(*s));
    memset(sinRemote, 0 ,sizeof(*sinRemote));

    // setting attribute of user socket
    s->sin_family = AF_INET;
    s->sin_addr.s_addr = htonl(INADDR_ANY);
    s->sin_port = htons(myPort);

    // set remote socket port with the remote port number user specified
    sinRemote->sin_port = htons(connectPort);

    // create socket descriptor
    int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    bind(socketDescriptor, (struct sockaddr*) s, sizeof(struct sockaddr_in));

    return socketDescriptor;
}

void Socket_close(int socketDescriptor) {
    close(socketDescriptor);
}