//To attribute: the Beej's guide and the video from class
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include "list.h"
#define MAX_LEN 256

//Structs
struct recv_args{
    int sock;
    int myP;
    int rturn;
    List* a_list;
};


struct send_args{
    int sock2;
    List* a_list;
    char* IP_add;
    int sturn;
    unsigned short port_add;
    //int firstturn;
};

//From section 6.3 of the manual - This is to get the socket address
//Gets socket Address:
void *get_in_addr(struct sockaddr* sa);

//Creates and bind UDP socket from given IP_address and port_number from user, returns UDP socket
int create_connection(char* IP_address, unsigned short port_number);

//RECEIVE MESSAGE: Receives the message and adds it to the list for printing later
void recv_msg(void* arguments);

//PRINTS MESSAGE: Prints the message and removes from list
void print_msg(void* arguments);

//FGETS FUNCTION: Adds user typed message and adds to the list 
//Will be used on the main thread
void keyboard_input(void* s_args);

//SEND MESSAGE: Sends it and removes from list
void send_msg(void* arguments2);

//This thread is used for printing the message that is received and removing it from the list
void prints_thread(struct recv_args* r_args);

//This thread is for receiving the message and adding it to the shared list
void recv_thread(struct recv_args* r_args);

//This thread is for sending the mesasge and removing it from the list
void sending_thread(struct send_args* s_args);
