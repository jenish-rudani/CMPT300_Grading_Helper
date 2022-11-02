#include<stdio.h>
#include<assert.h>
#include<stdlib.h>
#include<stddef.h>
#include "list.h"
#include<string.h>
#include<stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <netdb.h>
#include <poll.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>


/*
assignment2 BY ARJUN SINGh
most work was found using brian frasers tutorials or beejs socket and thread programming tutorials.
*/
//message length of the strings passed through
#define maxMsgLen 256

//list objects and strings
//list for host sent messages
List* senderList;
//list for receiving sent messages
List* receiverList;

//socket (same name as brian fraser socket variable from tutorial)
static int socketDescriptor;
//ipv4 structure
struct sockaddr_in send_sin, receive_sin;
//size of ip structures
//i got errors about signed and unsigned ints with these variables, so I set them to unsigned
unsigned int sendSinSize = sizeof(send_sin);
unsigned int receiveSinSize = sizeof(receive_sin);

//initializing port variables
static int sendingPort;
static int receivingPort;
//remote terminal name that we connect to
char* remoteName;

//pthreads
//from brian frasers tutorials on conditions and shutdowns
static pthread_mutex_t sMutex;
static pthread_mutex_t rMutex;
//for host sender signals when it is free and when to display message
static pthread_cond_t senderFree;
static pthread_cond_t senderBuffer;
//conditions of when a spot in the list is free or there is a buffer within the list with a message
//for receiver signals when it is free and when to display message
static pthread_cond_t receiverFree;
static pthread_cond_t receiverBuffer;

//FOR THESE 4 FUNCTIONS, I USED A LOT OF RESOURCES FROM BRIAN FRASERS TUTORIALS ON SOCKETS/PTHREADS AND CONDITIONS
//sender list input
//from hpc-tutorials.gov for structure of all functions
void *hostSenderUDP(void* socket){
	char storeMessage[maxMsgLen];
		while(1) {
			fgets(storeMessage, sizeof(storeMessage), stdin);
			//if the string inputted is !, exit the program, shut down and close are from brian frasers shutdown tutorial
			if(strcmp(storeMessage, "!\n") == 0) {
			printf("Bye bye\n");
			close(socketDescriptor);
			shutdown(socketDescriptor, SHUT_RDWR);
			exit(1);
		}
			if(storeMessage != NULL) {
				//lock in place until told otherwise
			pthread_mutex_lock(&sMutex);
			}
			//CHANGE TO LIST MAX
			if(List_count(senderList) > 100) {
				//wait until can proceed further
				pthread_cond_wait(&senderBuffer, &sMutex);
			}
			List_append(senderList, &storeMessage);
//signals for free send
			pthread_cond_signal(&senderFree);
//unlocks sending mutex
			pthread_mutex_unlock(&sMutex);
	}
}


//shows output on sender list
//from hpc-tutorials.gov for structure of all functions
void *hostSendPrint(void* socket){
	char storeMessage[maxMsgLen];
	memset(storeMessage, 0, maxMsgLen);
		while(1) {
			pthread_mutex_lock(&sMutex);
		if(List_count(senderList) == 0) {
			//if there is nothing in the list, send condition variables
			pthread_cond_wait(&senderBuffer, &sMutex);
		} else{
			List_first(senderList);
			strcpy(storeMessage, List_remove(senderList));
			//print last item removed from sender list
			printf("Message: %s\n", storeMessage);

			pthread_cond_signal(&senderFree);

			pthread_mutex_unlock(&sMutex);


		}
	}
}

//takes user input for receiver list
//from hpc-tutorials.gov for structure of all functions
void *receiverDestUDP(void* socket){
	char storeMessage[maxMsgLen];
	while(1) {
		//from beejs guide to sockets, https://beej.us/guide/bgnet/html/#socket - send() and recv()—Talk to me, baby!
			recvfrom(socketDescriptor, 
			storeMessage, 
			sizeof(storeMessage), 
			0, 
			((struct sockaddr *) &receive_sin), &receiveSinSize);
			pthread_mutex_lock(&rMutex);

			//if the string inputted is !, exit the program, shut down and close are from brian frasers shutdown tutorial
			if(strcmp(storeMessage, "!\n") == 0) {
			printf("Bye bye\n");
			close(socketDescriptor);
			shutdown(socketDescriptor, SHUT_RDWR);
			exit(1);
			}
			//here it makes it wait while the listn ode count is less than 100(the max)
			if(List_count(receiverList) > 100)  {
				pthread_cond_wait(&receiverBuffer, &rMutex);
			}
			List_append(receiverList, &storeMessage);

			pthread_cond_signal(&receiverFree);

			pthread_mutex_unlock(&rMutex);
	}
}

//printing out the messages for the receiver list
//from hpc-tutorials.gov for structure of all functions
void *receiverDestPrint(void* socket){
	char storeMessage[maxMsgLen];
	while(1) {
		pthread_mutex_lock(&rMutex);
		if(List_count(receiverList) == 0) {
			//if there is nothing in the list, send condition variables
			pthread_cond_wait(&receiverBuffer, &rMutex);
		}
		else {
			strcpy(storeMessage, List_remove(receiverList));

			//print output of last item removed from receiving list
			printf("Message: %s\n", storeMessage);

			pthread_cond_signal(&receiverFree);

			pthread_mutex_unlock(&rMutex);

		}
		
	}
}

int main(int argumentCount, char *arguments[]) {
	//checks if the argument list is even proper
	if(argumentCount != 4) {
		printf("Error, please use format: stalk [user port] [remote machine name] [remote port] ");
		return 0;
	}
	
	//from first tutorial on socket programming
	//sending port
	sendingPort = atoi(arguments[1]);
	//remote ports destination name
	remoteName = arguments[2];
	//receiving port
	receivingPort = atoi(arguments[3]);

//from brian fraser's tutorial on sockets
	//setting up the ipv4 for the sender and receiver
	//sender(source) address
	send_sin.sin_family = AF_INET;
	send_sin.sin_addr.s_addr = htonl(INADDR_ANY);
	send_sin.sin_port = htons(sendingPort);
	

	//receiver(destination) address 
	receive_sin.sin_family = AF_INET;
	receive_sin.sin_addr.s_addr = htonl(INADDR_ANY);
	receive_sin.sin_port = htons(receivingPort);

	socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
	//if socket doesn't work
	bind(socketDescriptor, (struct sockaddr *)&send_sin, sizeof(send_sin));

	printf("Connected!!\n ");
//if socket passes conditions
	//list created for host 
	senderList = List_create();
	//list created for receiver
	receiverList = List_create();
	
	printf("To leave stalk, enter '!'\n");

	//below is learned from brian fraser tutorials
	pthread_mutex_init(&sMutex, NULL);

	pthread_mutex_init(&rMutex, NULL);

	//for receiver signals when it is free and when to display message
	pthread_cond_init(&receiverFree, NULL);

	pthread_cond_init(&receiverBuffer, NULL);
	//for host sender signals when it is free and when to display message
	pthread_cond_init(&senderFree, NULL);

	pthread_cond_init(&senderBuffer, NULL);

	pthread_t threadOne;
	pthread_t threadTwo;
	pthread_t threadThree;
	pthread_t threadFour;

//start threads that work for each function dedicated.
	pthread_create(
		&threadOne,
		NULL,
		&hostSenderUDP,
		&socketDescriptor);

	pthread_create(
		&threadTwo,
		NULL,
		&hostSendPrint,
		&socketDescriptor);

	pthread_create(
		&threadThree,
		NULL,
		&receiverDestUDP,
		&socketDescriptor);

	pthread_create(
		&threadFour,
		NULL,
		&receiverDestPrint,
		&socketDescriptor);

//joining the threads, learned from brian frasers tutorial on threads 
	pthread_join(threadOne, NULL);
	pthread_join(threadTwo, NULL);
	pthread_join(threadThree, NULL);
	pthread_join(threadFour, NULL);


	return 0;
}