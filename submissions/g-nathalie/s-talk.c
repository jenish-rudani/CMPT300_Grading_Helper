#include "list.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>


#define MSG_MAX_LEN 1024

// data variables
struct sockaddr_in remoteSockAddress;
unsigned int remoteSockAddressLength  = sizeof(remoteSockAddress);

int socketDescriptor;

char message[MSG_MAX_LEN];

List *messageList;
List *messageList2;

bool endFlag = false;

// mutexes and condition variables
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condVar = PTHREAD_COND_INITIALIZER;
pthread_cond_t condVar2 = PTHREAD_COND_INITIALIZER;


//get remote machine IP address
char *getHostAddress(char *serverName){
	char *hostAddress = malloc(sizeof(hostAddress));
	struct hostent *host;
	struct in_addr **addressList;

	host = gethostbyname (serverName);
 	if (host != NULL) {
		addressList= (struct in_addr**) host->h_addr_list;
		for (int i = 0; addressList[i] != NULL; i++) {
			strcpy(hostAddress,inet_ntoa(*addressList[i]));
		}
  	} else {
    	printf("unable to get remote machine IP\n");
		exit(1);
	}
	return hostAddress;
}

void keyboardInputFunc() {
	while(1) {
		fgets(message, MSG_MAX_LEN, stdin);

		pthread_mutex_lock(&mutex1);
		pthread_cond_wait(&condVar, &mutex1);

		if (strcmp("!\n", message) == 0){
			endFlag = true;
		}

		if (List_prepend(messageList, &message) != 0){
			perror("unable to prepend to list");
			exit(1);
		}
		pthread_mutex_unlock(&mutex1);
	}
}

void screenOutputFunc() {
	while(1){
		pthread_mutex_lock(&mutex2);
		pthread_cond_wait(&condVar2, &mutex2);
		printf("%s", (char*)List_trim(messageList2));
		if (endFlag){
			pthread_cond_destroy(&condVar2);
			pthread_mutex_destroy(&mutex2);
			exit(0);
		}
		pthread_mutex_unlock(&mutex2);
	}
}

void UDPInputFunc() {
	while(1) {
		recvfrom(socketDescriptor,message, MSG_MAX_LEN, 0,
			(struct sockaddr *) &remoteSockAddress, &remoteSockAddressLength);
				
		pthread_mutex_lock(&mutex2);

		if (strcmp("!\n", message) == 0) {
			endFlag = true;
		}

		if (List_prepend(messageList2, &message) != 0){
			perror("unable to prepend to list");
			exit(1);
		}

		pthread_mutex_unlock(&mutex2);
		pthread_cond_signal(&condVar2);
		usleep(50);
	}
}

void UDPOutputFunc() {
	while(1) {
		pthread_mutex_lock(&mutex1);
		char* messageString = (char*) List_trim(messageList);
		sendto(socketDescriptor, messageString, MSG_MAX_LEN, 0, (struct sockaddr *) &remoteSockAddress, remoteSockAddressLength);
		pthread_mutex_unlock(&mutex1);
		if (endFlag){
			pthread_cond_destroy(&condVar);
			pthread_mutex_destroy(&mutex1);
			exit(0);
		}
		pthread_cond_signal(&condVar);
		usleep(50);
	}
}

int main(int argc, char *argv[]) {

	int portNumber = *(int*)argv[1];
	int remotePortNumber = *(int*)argv[3];

	socketDescriptor = socket(PF_INET, SOCK_DGRAM,0);
	if (socketDescriptor == -1) {
		perror("unable to initialize socket");
    	return EXIT_FAILURE;
	}

	remoteSockAddress.sin_family = AF_INET;
	remoteSockAddress.sin_addr.s_addr = inet_addr(getHostAddress(argv[2]));
	remoteSockAddress.sin_port = htons(remotePortNumber);

	struct sockaddr_in sockAddress;
	memset(&sockAddress, 0, sizeof(sockAddress));
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	sockAddress.sin_port = htons(portNumber);

	if (bind (socketDescriptor, (struct sockaddr*) &sockAddress, sizeof(sockAddress)) == -1){
		perror("unable to bind socket");
    	return EXIT_FAILURE;
	}

	messageList = List_create();
	messageList2 = List_create();

	// create and join pthreads
	pthread_t UDPOutputThread, UDPInputThread, keyboardInputThread, screenOutputThread;

	pthread_create(&UDPOutputThread, NULL, (void*) &UDPOutputFunc, "");
	pthread_create(&UDPInputThread, NULL, (void*) &UDPInputFunc, "");
	pthread_create(&keyboardInputThread, NULL, (void*) &keyboardInputFunc, "");
	pthread_create(&screenOutputThread, NULL, (void*) &screenOutputFunc, "");

	pthread_join(UDPOutputThread, NULL);
	pthread_join(UDPInputThread, NULL);
	pthread_join(keyboardInputThread, NULL);
	pthread_join(screenOutputThread, NULL);
}