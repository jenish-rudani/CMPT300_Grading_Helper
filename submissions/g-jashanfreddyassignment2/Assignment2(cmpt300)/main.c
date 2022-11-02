#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "list.h"
#include "s-talk.h"

int main(int argc, char* argv[])
{
    if(argc != 4)
	{
		puts("Client: Re-enter arguments\n");
		exit(1);
	}

    //Creates the list
    initializingLists();
  
    NetworkingInfo info;
	info.clientPort = argv[1];
    info.remoteMachinename = argv[2];
    info.remotePort = argv[3];

    //Initializes the socket
    info.socket_results = initializingSocket(&info);

    //creates the threads
    initializingThreads(info);
   
    return 0;
}