#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include "socket.h"

static int s_socketDescriptor;
static struct sockaddr_in s_sinRemote;


// Open an IPV4 socket for listening
//   `localPortStr':  "12345"
static int openUdpIpv4Socket(char *localPortStr) 
{
	// From: http://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch
	// Setup restrictions (hints) for IP info
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET;       // don't care (AF_UNSPEC) IPv4 (AF_INET) or IPv6 (AF_INET6)
	hints.ai_socktype = SOCK_DGRAM;  // UDP (SOCK_DGRAM) TCP(SOCK_STREAM)
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	struct addrinfo *servinfo;       // will point to the results; allocated by getaddrinfo
	int status = getaddrinfo(NULL, localPortStr, &hints, &servinfo);
	if (status != 0) {
		fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerror(status));
		exit(1);
	}

	// If you are looking for different options, loop through them!
	for(struct addrinfo *p = servinfo;p != NULL; p = p->ai_next) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
		void *addr = &(ipv4->sin_addr);
		
		// convert the IP to a string and print it:
		char targetIdStr[INET6_ADDRSTRLEN];
		inet_ntop(p->ai_family, addr, targetIdStr, INET6_ADDRSTRLEN);
		// printf("  Listen IP %s\n", targetIdStr);
	}

	// Create the socket for UDP and bind
	int socketDescriptor = socket(
        servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (socketDescriptor == -1) {
		fprintf(stderr, "socket() error: %s\n", strerror(errno));
		exit(1);
	}

	status = bind(socketDescriptor, servinfo->ai_addr, servinfo->ai_addrlen);
	if (status == -1) {
		fprintf(stderr, "socket() error: %s\n", strerror(errno));
		exit(1);
	}

	// Cleanup: free the linked-list
	freeaddrinfo(servinfo);

	return socketDescriptor;
}

// Setup socetaddr_in data for sending to a target
static void populateRemoteSocketAddrInfo(
    char *remoteNameOrIpStr,
    char *remotePortStr, 
    struct sockaddr_in *pSocketAddrInfo)
{
    	// From: http://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch
	// Setup restrictions (hints) for IP info
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET;       // don't care (AF_UNSPEC) IPv4 (AF_INET) or IPv6 (AF_INET6)
	hints.ai_socktype = SOCK_DGRAM;  // UDP (SOCK_DGRAM) TCP(SOCK_STREAM)
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	struct addrinfo *servinfo;       // will point to the results; allocated by getaddrinfo
	int status = getaddrinfo(remoteNameOrIpStr, remotePortStr, &hints, &servinfo);
	if (status != 0) {
		fprintf(stderr, "Remote getaddrinfo() error: %s\n", gai_strerror(status));
		exit(1);
	}

	// If you are looking for different options, loop through them!
	for(struct addrinfo *p = servinfo;p != NULL; p = p->ai_next) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
		void *addr = &(ipv4->sin_addr);
		
		// convert the IP to a string and print it:
		char targetIdStr[INET6_ADDRSTRLEN];
		inet_ntop(p->ai_family, addr, targetIdStr, INET6_ADDRSTRLEN);
		// printf("  Remote IP %s\n", targetIdStr);

        memcpy(pSocketAddrInfo, ipv4, sizeof(*pSocketAddrInfo));
	}

    // Cleanup: free the linked-list
	freeaddrinfo(servinfo);
}

void Socket_init(char* pLocalPort, char* pRemoteHost, char *pRemotePort)
{
    s_socketDescriptor = openUdpIpv4Socket(pLocalPort);
    populateRemoteSocketAddrInfo(pRemoteHost, pRemotePort, &s_sinRemote);
}

// Blocks until a message is received.
// Returns number of bytes written into pMessage (range [0, maxLength])
int Socket_receiveBlocking(char *pMessage, int maxLength)
{
    // get the data (blocking)
    // will change sin (the address) to be the address of the client.
    // note: sin passes information in and out of call!
    struct sockaddr_in sinRemote;
    unsigned int sin_len = sizeof(sinRemote);
    int bytesRx = recvfrom(s_socketDescriptor,
        pMessage, maxLength-1, 0,
        (struct sockaddr *) &sinRemote, &sin_len);

    // make it null terminated (so string functions work):
    pMessage[bytesRx] = 0;
    
    // long remotePort = ntohs(sinRemote.sin_port);
    // printf("(Port %ld) rx: '%s'\n", remotePort, pMessage);

    return bytesRx;
}

// Send pMessage (length = bytes to send)
// Returns number of bytes sent
int Socket_sendMessage(char *pMessage, int length)
{
    // transmit message
    unsigned int sin_len = sizeof(s_sinRemote);

    int bytesSent = sendto(s_socketDescriptor,
        pMessage, length,
        0,
        (struct sockaddr *) &s_sinRemote, sin_len);

    return bytesSent;
}

void Socket_shutdown()
{
    // TOOD: error check
   	close(s_socketDescriptor);
}
