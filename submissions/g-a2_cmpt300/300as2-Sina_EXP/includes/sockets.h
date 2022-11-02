#ifndef SOCKETS_H
#define SOCKETS_H

#include "sockets.h"
#include "receiver.h"
#include "sender.h"
#include "list.h"
#include <pthread.h>
#include <stdio.h>
#include "arpa/inet.h"
#include <stdlib.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

// Start background receive thread
void Sockets_init(char *rport, char *remoteCPU, char *sport);

// Stop background receive thread and cleanup
void Sockets_shutdown();

#endif
