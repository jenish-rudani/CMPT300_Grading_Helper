#ifndef HEADER_UTILS
#define HEADER_UTILS
#include <pthread.h>

	int hostname_to_ip(char *, char *);
	pthread_mutex_t lock,lockrx;
	pthread_cond_t cond,condrx;

	struct sockaddr_in theiraddr, myaddr;
	int done,donerx;
	int sockfd;
	
	int ch;
	#define MAXSIZE 8192
	#define MAX 20


#endif
