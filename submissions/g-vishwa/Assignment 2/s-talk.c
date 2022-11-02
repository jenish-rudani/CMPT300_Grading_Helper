/* Author: Vishwa Venkateshwaran */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

int const STRINGLEN = 256; 

List *inputSenderList;
List *outputRecieverList;

int sockfd;
int rv;
struct addrinfo hints, *servinfoL, hintsTalker, *servinfo;
struct sockaddr_in their_addr;
socklen_t addr_len;

pthread_mutex_t inputSenderMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t outputRecieverMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t senderBuffer = PTHREAD_COND_INITIALIZER;
pthread_cond_t recieverBuffer = PTHREAD_COND_INITIALIZER;

void closeEverything() {
	freeaddrinfo(servinfo);
	freeaddrinfo(servinfoL);
	close(sockfd);
	pthread_mutex_destroy(&inputSenderMutex);
	pthread_mutex_destroy(&outputRecieverMutex);
	List_free(inputSenderList, free);
	List_free(outputRecieverList, free);
	exit(1);
}

void *inputThread(void *whatAmISupposedToDoWithThis) {
	char *messageRead;
	while(1) {
		messageRead = malloc(STRINGLEN*sizeof(char));
		int returnVal = read(0, messageRead, STRINGLEN);
		returnVal = pthread_mutex_lock(&inputSenderMutex);
		if (returnVal != 0) {
			free(messageRead);
			perror("An error has occurred with a mutex.\n");
			pthread_mutex_unlock(&inputSenderMutex);
			closeEverything();
		}
		returnVal = List_prepend(inputSenderList, messageRead);

		if (returnVal != 0) {
			free(messageRead);
			perror("An error has occurred with the list.\n");
			pthread_mutex_unlock(&inputSenderMutex);
			closeEverything();
		}
		
		pthread_cond_signal(&senderBuffer);
		pthread_mutex_unlock(&inputSenderMutex);
	}
}

void *senderThread(void *whatAmISupposedToDoWithThis){
	char *messageRead;
	while(1) {
		int returnVal = pthread_mutex_lock(&inputSenderMutex);
		if (returnVal != 0) {
			perror("An error has occurred with a mutex.\n");
			pthread_mutex_unlock(&inputSenderMutex);
			closeEverything();
		}
	
		if (List_count(inputSenderList) == 0) {
			pthread_cond_wait(&senderBuffer, &inputSenderMutex);
		}

		while(List_count(inputSenderList) != 0) {
			messageRead = malloc(STRINGLEN*sizeof(char));
			strcpy(messageRead, (char*) List_trim(inputSenderList));
			
			if (sendto(sockfd, messageRead, strlen(messageRead), 0, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
				free(messageRead);
				perror("Error when sending the message.\n");
				pthread_mutex_unlock(&inputSenderMutex);
				closeEverything();
			}

			if (strcmp(messageRead, "!\n") == 0) {
				free(messageRead);
				pthread_mutex_unlock(&inputSenderMutex);
				closeEverything();
			}
			free(messageRead);
			messageRead = NULL;
		}
		pthread_mutex_unlock(&inputSenderMutex);
	}
}

void *outputThread(void *whatAmISupposedToDoWithThis){
	char *messageRead;
	while(1){
		int returnVal = pthread_mutex_lock(&outputRecieverMutex);
		if (returnVal != 0) {
			perror("An error has occurred with a mutex.\n");
			pthread_mutex_unlock(&inputSenderMutex);
			closeEverything();
		}

		if (List_count(outputRecieverList) == 0) {
			pthread_cond_wait(&recieverBuffer, &outputRecieverMutex);
		}
		
		while(List_count(outputRecieverList) != 0) {
			messageRead = malloc(STRINGLEN*sizeof(char));
			strcpy(messageRead, (char*) List_trim(outputRecieverList));
			
			write(1, messageRead, strlen(messageRead));; 
			if (strcmp(messageRead, "!\n") == 0) {
				free(messageRead);
				pthread_mutex_unlock(&inputSenderMutex);
				closeEverything();
			}
			free(messageRead);
			messageRead = NULL;
		}
		pthread_mutex_unlock(&outputRecieverMutex);
	}
}

void *recieverThread(void *whatAmISupposedToDoWithThis){
	char *messageRead = malloc(STRINGLEN*sizeof(char));
	while(1) {
		addr_len = sizeof(their_addr);
		while(recvfrom(sockfd, messageRead, STRINGLEN, 0, (struct sockaddr*)&their_addr, &addr_len) != -1) {
			int returnVal = pthread_mutex_lock(&outputRecieverMutex);
			if (returnVal != 0) {
				perror("An error has occurred with a mutex.\n");
				pthread_mutex_unlock(&outputRecieverMutex);
				pthread_cond_signal(&recieverBuffer);
				closeEverything();
			}
			
			returnVal = List_prepend(outputRecieverList, messageRead);
			if (returnVal != 0) {
				free(messageRead);
				perror("An error has occurred with the list.\n");
				pthread_mutex_unlock(&outputRecieverMutex);
				pthread_cond_signal(&recieverBuffer);
				closeEverything();
			}

			pthread_cond_signal(&recieverBuffer);
			pthread_mutex_unlock(&outputRecieverMutex);
			messageRead = malloc(STRINGLEN*sizeof(char));
		}
	}
}

int main(int argc, char *argv[]){
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	memset(&hintsTalker, 0, sizeof(hintsTalker));
	hintsTalker.ai_family = AF_UNSPEC;
	hintsTalker.ai_socktype = SOCK_DGRAM;
	
	if((rv = getaddrinfo(NULL, argv[1], &hints, &servinfoL)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	if((sockfd = socket(servinfoL->ai_family, servinfoL->ai_socktype, servinfoL->ai_protocol)) == -1){
		perror("listener: socket");
	} 
	
	if(bind(sockfd, servinfoL->ai_addr, servinfoL->ai_addrlen) == -1){
		close(sockfd);
		perror("listner: bind");
	}
	
	if((rv = getaddrinfo(argv[2], argv[3], &hintsTalker, &servinfo) != 0)) {
		fprintf(stderr, "getaddrinfo (out): %s\n", gai_strerror(rv));
		return 1;
	}
	
	pthread_t p_outputThread, p_inputThread, p_senderThread, p_recieverThread;

	inputSenderList = List_create();
	outputRecieverList = List_create();
	
	pthread_create(&p_outputThread, NULL, outputThread, NULL);
	pthread_create(&p_inputThread, NULL, inputThread, NULL);
	pthread_create(&p_senderThread, NULL, senderThread, NULL);
	pthread_create(&p_recieverThread, NULL, recieverThread, NULL);

	pthread_join(p_outputThread, NULL);
	pthread_join(p_inputThread, NULL);
	pthread_join(p_senderThread, NULL);
	pthread_join(p_recieverThread, NULL);

	return 0;
}
