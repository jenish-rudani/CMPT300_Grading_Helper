// Project done in pairs
// Matthew Wells | 301 396 858 | mrwells@sfu.ca
// Tal Zaloilov | 312 838 774 | tzaloilo@sfu.ca

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "list.h"

#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>  
#include <netdb.h>   

// Maximum size (in bytes) of messages to be send over the network.
const int MESSAGE_SIZE_LIMIT = 256;

// Define functions and threads
void *keyboardInput(void* outgoingData);
void *datagramInput(void *incomingData);
void *screenOutput(void *incomingData);
void *datagramOutput(void *outgoingData);
void bringToMainMenu();
void console_print(char* msg);

// Define mutex locks / conditions
pthread_mutex_t outgoingLock;
pthread_mutex_t incomingLock;

pthread_mutex_t outgoingEmptyLock;
pthread_mutex_t incomingEmptyLock;

pthread_cond_t outgoingCondition;
pthread_cond_t incomingCondition;

// Define IDs for keeping track of threads
pthread_t keyboardInputID;
pthread_t datagramInputID;
pthread_t screenOutputID;
pthread_t datagramOutputID;

// Define sockets
int client_socket;
int server_socket;

struct sockaddr_in this_server_address; // Holds IP and port of this computer's server
struct sockaddr_in partner_server_address; // Holds IP and port of the partner's server
struct sockaddr_in partner_client_address; // Holds IP and port of the computer client connected to this computer

int client_struct_length;
int server_struct_length;

// Code that initializes all this computer's server UDP stuff. The partner's computer will connect to this port to interface with the server here.
// Returns 0 on success, -1 on fail.
int initialize_server_udp(int selfPort) {

    client_struct_length = sizeof(partner_client_address);
    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (server_socket < 0) {
        console_print("Something went wrong when trying to create the server socket.\n");
        return -1;
    }

    // Set port and IP:
    this_server_address.sin_family = AF_INET;
    this_server_address.sin_port = htons(selfPort);
    this_server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // This is the local address of this computer

    // Bind to the set port and IP:
    if (bind(server_socket, (struct sockaddr*)&this_server_address, sizeof(this_server_address)) < 0){
        console_print("Couldn't bind to the port. Most likely, there is something already running on that port.\n");
        close(server_socket);
        return -1;
    }
    return 0;
}

// Code that initializes all the client UDP stuff. So this is the socket that will connect to the partner computer's port.
// Return 0 on success, -1 on fail
int initialize_client_udp(int partnerPort, char* partnerComputerAddress) {

    server_struct_length = sizeof(partner_server_address);
    // Create socket:
    client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(client_socket < 0){
        console_print("Error while creating client socket. \n");
        close(server_socket); // Close server socket here, as it was created before this client socket
        return -1;
    }
    
    // Set port and IP:
    partner_server_address.sin_family = AF_INET;
    partner_server_address.sin_port = htons(partnerPort);
    partner_server_address.sin_addr.s_addr = inet_addr(partnerComputerAddress);
    return 0;
}

// Takes input from keyboard
void *keyboardInput(void* outgoingData) {
    while(1) {
        char *input = (char *) malloc(MESSAGE_SIZE_LIMIT);
        fgets(input, MESSAGE_SIZE_LIMIT, stdin);

        if (input[0] == '\n') {
            free(input);
            fputs("You cannot enter an empty value.\n", stdout);
        } else {
            input[strlen(input) - 1] = '\0';

            // Handle Critical Section
            pthread_mutex_lock(&outgoingLock);
            List_append(outgoingData, input);
            pthread_mutex_unlock(&outgoingLock);

            // If the output is waiting because it has nothing to process then continue it
            pthread_mutex_lock(&outgoingEmptyLock);
            if (List_count(outgoingData) == 1) {
                pthread_cond_signal(&outgoingCondition);
            }
            pthread_mutex_unlock(&outgoingEmptyLock);
        }
    }
}

// Awaits arrival of datagrams from other user
void *datagramInput(void *incomingData) {
    // Continually waits for datagrams to come in
    while(1) {
        // Recieve message from the client 

        char *client_message = malloc(MESSAGE_SIZE_LIMIT);
        if (recvfrom(server_socket, client_message, MESSAGE_SIZE_LIMIT, 0,
            (struct sockaddr*)&partner_client_address, &client_struct_length) <= 0){
            console_print("Something went wrong when receiving last message.\n");
        } else {

            // Stop program
            if (strcmp("!", client_message) == 0){
                free(client_message);
                bringToMainMenu();
                break;
            } 

            pthread_mutex_lock(&incomingLock);
            // Add received data to incomingData list 
            List_append(incomingData, client_message);        
            pthread_mutex_unlock(&incomingLock);

            // If this is the only unprocessed input then unlock screenOutput to print it to the screen
            pthread_mutex_lock(&incomingEmptyLock);
            if (List_count(incomingData) == 1) {
                pthread_cond_signal(&incomingCondition);
            }
            pthread_mutex_unlock(&incomingEmptyLock);
        }
    }
}

// prints information received from other user to screen
void *screenOutput(void *incomingData) {
    // Continually print recieved messages to the screen
    while(1) {    
        // If there are no messages to be sent then wait
        pthread_mutex_lock(&incomingEmptyLock);
        if (List_count(incomingData) == 0) {
            pthread_cond_wait(&incomingCondition, &incomingEmptyLock);
        }
        pthread_mutex_unlock(&incomingEmptyLock);

        // Prints first element of the incomingMessages to the screen, deallocates space, and removes from list
        pthread_mutex_lock(&incomingLock);
        List_first(incomingData);
        fputs(List_curr(incomingData), stdout);
        printf("\n");
        free(List_remove(incomingData));
        pthread_mutex_unlock(&incomingLock);
    }
}

// sends keyboard inputs out to other user (list values that were sent out by the keyboardInput thread)
void *datagramOutput(void *outgoingData) {
    // Continually send messages to other user

    while(1) {
        // If there are no messages to be sent then wait for a message to be added
        pthread_mutex_lock(&outgoingEmptyLock);
        if (List_count(outgoingData) == 0) {
            pthread_cond_wait(&outgoingCondition, &outgoingEmptyLock);
        }
        pthread_mutex_unlock(&outgoingEmptyLock);
        
        // Handle Critical Section
        pthread_mutex_lock(&outgoingLock);

        List_first(outgoingData);
        char *client_message; 
        client_message = List_curr(outgoingData);
        // Send the message to server by using the server's port and address:

        // Sends strlen(client_message) amount of bytes.
        if (sendto(client_socket, List_curr(outgoingData), strlen(List_curr(outgoingData)), 0,
            (struct sockaddr*)&partner_server_address, server_struct_length) < 0){
            console_print("Unable to send message. Please try again.\n");
        }

        // Stop program
        if (strcmp("!", List_curr(outgoingData)) == 0){
            free(List_remove(outgoingData));
            bringToMainMenu();
            break;
        } 

        free(List_remove(outgoingData));
        pthread_mutex_unlock(&outgoingLock);
    }
}

void threadsSetup() {
    // Create 2 lists, one for printing data, one for sending data
    List* outgoingData = List_create();
    List* incomingData = List_create();

    // Initialize mutex locks / conditions
    pthread_mutex_init(&outgoingEmptyLock, NULL);
    pthread_mutex_init(&incomingEmptyLock, NULL);

    pthread_mutex_init(&outgoingLock, NULL);
    pthread_mutex_init(&incomingLock, NULL);

    pthread_cond_init(&outgoingCondition, NULL);
    pthread_cond_init(&incomingCondition, NULL);

    // Open all threads
    pthread_create(&keyboardInputID, NULL, keyboardInput, (List*) outgoingData);
    pthread_create(&datagramInputID, NULL, datagramInput, (List*) incomingData);
    pthread_create(&screenOutputID, NULL, screenOutput, (List*) incomingData);
    pthread_create(&datagramOutputID, NULL, datagramOutput, (List*) outgoingData);

    // Close all threads
    pthread_join(keyboardInputID, NULL);
    pthread_join(datagramInputID, NULL);
    pthread_join(screenOutputID, NULL);
    pthread_join(datagramOutputID, NULL);
}

void closeProgram(){
    // Close all mutex locks / conditions
    pthread_mutex_destroy(&outgoingEmptyLock);
    pthread_mutex_destroy(&incomingEmptyLock);

    pthread_mutex_destroy(&outgoingLock);
    pthread_mutex_destroy(&incomingLock);

    pthread_cond_destroy(&outgoingCondition);
    pthread_cond_destroy(&incomingCondition);
}

void console_print(char* msg) {
    fputs(msg, stdout);
}

void bringToMainMenu(){
    // Close sockets
    console_print("Disconnected from partner and the s-talk session has been ended\n.");
    close(client_socket);
    close(server_socket);

    // Cancel threads
    pthread_cancel(keyboardInputID);
    pthread_cancel(datagramInputID);
    pthread_cancel(screenOutputID);
    pthread_cancel(datagramOutputID);
}

char* nameToIP(char *hostname)
{
	struct hostent *hostEnt;
	struct in_addr **addresses;
    char *address = malloc(256);

	if ((hostEnt = gethostbyname(hostname)) == NULL) 
	{
        free(address);
		return NULL;
	}

	addresses = (struct in_addr **) hostEnt->h_addr_list;
	strcpy(address, inet_ntoa(*addresses[0])); // This copies and returns the first result
    return address;
}

int main() {
    char initialUserInput[50];
    char *userInputArray[40];

    while (1)
    {
        int i = 0;

        console_print("To start s-talk program, enter input in form 's-talk [your port address] [partner computer name] [partner port address].'\n");
        fgets(initialUserInput, 50, stdin);
        
        char *splitString = strtok(initialUserInput, " ");
        while (splitString != NULL) {
            userInputArray[i++] = splitString;
            splitString = strtok(NULL, " ");
        }

        // Check if input is valid
        if ((strcmp(userInputArray[0], "s-talk") == 0) && userInputArray[1] != NULL && userInputArray[2] != NULL && userInputArray[3] != NULL)
        {
            char *ipAddress = nameToIP(userInputArray[2]);
            if (ipAddress != NULL){

                // If server init failed, don't init client and try again
                if (initialize_server_udp(atoi(userInputArray[1])) == 0){
                    initialize_client_udp(atoi(userInputArray[3]), ipAddress); // the port the partner's computer will be on
                    threadsSetup();
                } 
            } else {
                console_print("That is not a valid partner computer name. Please try again.");
            }
            free(ipAddress);
        } 

    }
    return 0;
}
