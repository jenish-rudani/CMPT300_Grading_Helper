#ifndef _SOCKET_HANDLER_H_
#define _SOCKET_HANDLER_H_
#include <netdb.h>
#include <stdbool.h>

// Check if the port number is valid "1024 < port number < 65535"
// Other ports are reserved according to beejs guide (Chapter 5 - bind() - page 26)
bool isValidPort(int portNumber);

// Sets up the remote Address information from the given arguments to send data to
bool setUpRemoteAddressInfo(char* machineName, char* machinePortNumber);

// Create the local socket to receive data on
int createMyLocalSocket(int portNumber);

// Getter for the local socket value used for sendto and recvfrom
int getLocalSocketValue();

// Getter for the remote machine address which will contain the required information for sentto
struct addrinfo* getRemoteMachineAddress();

// Shutdown the threads and socket while also calling clean up Lists
void shutDownSocket();

#endif