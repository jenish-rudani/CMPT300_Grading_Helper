#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <netdb.h>

int Socket_init(int port, int connectPort, struct sockaddr_in* s, struct sockaddr_in* sinRemote);

void Socket_close(int socketDescriptor);

#endif