// Create a UDP server which listens on a given port, and which
// sends to a given remote-host / port

#ifndef _SOCKET_H_
#define _SOCKET_H_

void Socket_init(char* pLocalPort, char* pRemoteHost, char *pRemotePort);
void Socket_shutdown(void);

// Blocks until a message is received.
// Returns number of bytes written into pMessage (range [0, maxLength])
int Socket_receiveBlocking(char *pMessage, int maxLength);

// Send pMessage (length bytes)
// Returns number of bytes sent
int Socket_sendMessage(char *pMessage, int length);

#endif