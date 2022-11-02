#include "list.h"

typedef struct NetworkingInfo NetworkingInfo;
struct NetworkingInfo{
	char* clientPort;
    char* remoteMachinename;
    char* remotePort;
    struct addrinfo* socket_results;
};

void initializingLists(void);
void* initializingThreads(NetworkingInfo info);

struct addrinfo* initializingSocket(void* info);

void* receiveThread(void* info);
void* sendThread(void* info);
void* getinputThread();
void* printOutputThread();




