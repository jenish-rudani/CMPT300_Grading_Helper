#include "threads.h"


static pthread_mutex_t  keyboardSendListMutex = PTHREAD_MUTEX_INITIALIZER;  
static pthread_mutex_t  printAndReceiveMutex = PTHREAD_MUTEX_INITIALIZER; 
static pthread_mutex_t  endProgramMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t endProgramCondVar = PTHREAD_COND_INITIALIZER;

static pthread_t keyboardThread;
static pthread_t sendThread;
static pthread_t ReceiveThread;
static pthread_t printThread;

void* initThreads(void* unused){

		getPorts();
		List* keyboardAndOutputList = List_create(); //reads keyboard and send data
		List* screenAndInputList = List_create(); // print to screen and receive data
		pthread_create(&keyboardThread, NULL, keyboardControl, keyboardAndOutputList);
		pthread_create(&sendThread, NULL, sendControl, keyboardAndOutputList);
		pthread_create(&ReceiveThread, NULL, receiveControl, screenAndInputList);
		pthread_create(&printThread, NULL, printControl, screenAndInputList);

		//wait to until signalled to end program
		pthread_mutex_lock(&endProgramMutex);
		{
		pthread_cond_wait(&endProgramCondVar, &endProgramMutex);
		}
		pthread_mutex_unlock(&endProgramMutex);

		pthread_cancel(keyboardThread);
		pthread_cancel(sendThread);
		pthread_cancel(ReceiveThread);
		pthread_cancel(printThread);

		return NULL;
}
 

void getPorts(){

	char myPortString[1024];
	char getPortData[1024];
	char sTalk[1024];
	int sections = 0;
	int index = 0;

	printf("Enter Start Values ('s-talk [YOUR PORT] [OTHER USER MACHINE NAME] [OTHER USER PORT]')\n");

	while(1){

		//reset all values after failed input
		memset(getPortData, 0, sizeof(getPortData));
		memset(myPortString, 0, sizeof(myPortString));
		memset(friendPortString, 0, sizeof(friendPortString));
		memset(machineName, 0, sizeof(machineName));
		memset(sTalk, 0, sizeof(sTalk));

		fgets(getPortData, 1024, stdin);


			sections = 0;
			index = 0;

			//seperate values in string 
			for(int i = 0; i < strlen(getPortData); i++){

				if(getPortData[i] == ' '){
					sections++;
					index = 0;
					continue;
				}
				else if(getPortData[i] == '\n' || getPortData[i] == '\0'){
			
					break;
				}
				if(sections == 0){
					sTalk[index] = getPortData[i];
			
				}
				else if(sections == 1){
					myPortString[index] = getPortData[i];
				
				}
				else if(sections == 2){
					machineName[index] = getPortData[i];
		
					
				}
				else{
					friendPortString[index] = getPortData[i];
				
				}
				index++;
			}

			if(!strcmp(sTalk, "s-talk")){
				myPort = atoi(myPortString);
				friendPort = atoi(friendPortString);
				printf("**StARTING CHAT SESSION**\n");
				break;
			}

			printf("INVALID INPUT PLEASE TRY AGAIN\n");
		
	}

}



void* keyboardControl(void* sharedList){

	while(1){
	
		char* currentMessage = (char*)malloc(1024);

		fgets(currentMessage, 1024, stdin);

		//if the no char is entered don't send
		if(strlen(currentMessage) == 1){
		
			free(currentMessage);
			continue;
		}

		//add item to list
		pthread_mutex_lock(&keyboardSendListMutex);
		{
		List_append((List*)sharedList, currentMessage);
		}
		pthread_mutex_unlock(&keyboardSendListMutex);
	}

	return NULL;
}

void* sendControl(void* sharedList){

	struct addrinfo* remoteIPAddr;

	//get ip of remote machine using its name and port
	if(getaddrinfo(machineName, friendPortString, NULL, &remoteIPAddr) == 0){
		sockaddr_in *tempAddr = (struct sockaddr_in *)remoteIPAddr->ai_addr;

	sendSocket = socket(PF_INET, SOCK_DGRAM, 0);
	memset(&remoteSendAddr, 0, sizeof(remoteSendAddr));
	remoteSendAddr.sin_family = AF_INET;
	remoteSendAddr.sin_addr.s_addr = inet_addr(inet_ntoa(tempAddr->sin_addr));
	remoteSendAddr.sin_port = htons(friendPort);
	}
	else{

			printf("**COULD NOT GET IP OF OTHER USER**\n");
			printf("**NOW ENDING THE SESSION**\n");
				
				pthread_mutex_lock(&endProgramMutex);
				{
				pthread_cond_signal(&endProgramCondVar);
				}
				pthread_mutex_unlock(&endProgramMutex);

	}

	while(1){

		
		pthread_mutex_lock(&keyboardSendListMutex);
		{
		if(List_count((List*) sharedList) > 0){

			char *currentMessage = (char*)List_curr((List*)sharedList);

			int remoteSendLen = sizeof(remoteSendAddr);
		
			//send the message
			sendto(sendSocket, currentMessage, strlen(currentMessage), 0, (struct sockaddr *) &remoteSendAddr, remoteSendLen);

			//if the current messsage is "!" signal the cancel con varibale to end the program
			if(!strncmp(currentMessage, "!", 1) && strlen(currentMessage) == 2){

				free(remoteIPAddr);
				free(currentMessage);
				List_remove((List*) sharedList);
				printf("**NOW ENDING THE SESSION**\n");
				
				pthread_mutex_lock(&endProgramMutex);
				{
				pthread_cond_signal(&endProgramCondVar);
				}
				pthread_mutex_unlock(&endProgramMutex);
			}
			else{
				free(currentMessage);
				List_remove((List*) sharedList);
			}

		

			
		}
		}
		pthread_mutex_unlock(&keyboardSendListMutex);
		
	


	}
	return NULL;

}


void* receiveControl(void * sharedList){

	//initialize socket and adderess as well as bind the 2
	memset(&myReceiveAddr, 0, sizeof(myReceiveAddr));
	myReceiveAddr.sin_family = AF_INET;
	myReceiveAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myReceiveAddr.sin_port = htons(myPort);

	receiveSocket = socket(PF_INET, SOCK_DGRAM, 0);
	bind (receiveSocket, (struct sockaddr*) &myReceiveAddr, sizeof(myReceiveAddr));

	struct sockaddr_in remoteReceiveAddr;
	unsigned int sin_len = sizeof(remoteReceiveAddr);
	
	while(1){
		
		//receive message and add it to the list
		char* receivedMesage = (char*)malloc(1024);
		memset(receivedMesage, 0, sizeof(receivedMesage));
		recvfrom(receiveSocket, receivedMesage, 1024, MSG_WAITALL, (struct sockaddr *) &remoteReceiveAddr, &sin_len);
		
		pthread_mutex_lock(&printAndReceiveMutex);
		{
		List_append((List*) sharedList, receivedMesage);
			}
		pthread_mutex_unlock(&printAndReceiveMutex);
	
	}

	return NULL;
}




void* printControl(void* sharedList){


	while(1){

		pthread_mutex_lock(&printAndReceiveMutex);
		{
		if(List_count((List*) sharedList) > 0){
		char *printMessage = (char*)List_curr((List*)sharedList);

		//if the current message is "!" do not print it and signal con variable to end the program
		if(!strncmp(printMessage, "!", 1) && strlen(printMessage) == 2){

			free(printMessage);
			List_remove((List*) sharedList);
			printf("\n%s, has exited the chat session. \n**NOW ENDING THE SESSION**\n", machineName);
				
			pthread_mutex_lock(&endProgramMutex);
			{
			pthread_cond_signal(&endProgramCondVar);
			}
			pthread_mutex_unlock(&endProgramMutex);
		}
		else{

			//prints the message and remove it from list
			fputs(printMessage, stdout);
			free(printMessage);
			List_remove((List*) sharedList);
		}

		}
		}
		pthread_mutex_unlock(&printAndReceiveMutex);


	}
	return NULL;
}



