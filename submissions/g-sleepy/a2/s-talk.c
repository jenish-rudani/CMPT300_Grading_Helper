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

#include "s-talk.h"
#include "list.h"
#define MAX_LEN 256

//Threads
pthread_t fget_thread, rcv_thread, print_thread, send_thread;

//MUTEX/CONDITIONS

//Goes into receive function, signalling that it is ok to print
pthread_cond_t ok_to_print = PTHREAD_COND_INITIALIZER;
pthread_mutex_t ok_to_print_mutex = PTHREAD_MUTEX_INITIALIZER;

//Goes into keyboard function, signalling that it is okay to send
pthread_cond_t ok_to_send = PTHREAD_COND_INITIALIZER;
pthread_mutex_t ok_to_send_mutex = PTHREAD_MUTEX_INITIALIZER;


//From section 6.3 of the manual - This is to get the socket address
//Gets socket Address:
void *get_in_addr(struct sockaddr* sa)
{
    //If in IPV4
    if (sa->sa_family == AF_INET) {
            return &(((struct sockaddr_in*)sa)->sin_addr);
        }
    //If in IPV6 - probably won't need
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//Creates and bind UDP socket from given IP_address and port_number from user, returns UDP socket
int create_connection(char* IP_address, unsigned short port_number){
    //Create sockaddr_in struct
    struct sockaddr_in sock;
    memset(&sock, 0, sizeof(sock));
    sock.sin_family = AF_INET;

    //sin_addr is another struct (why there's another inside)
    sock.sin_addr.s_addr = inet_addr(IP_address);
    sock.sin_port = htons(port_number);

    //Create UPD socket (PF_INET could be replaced with AF_INET)
    int UDP_socket = socket(PF_INET, SOCK_DGRAM, 0);

    //Bind the socket to port
    int error = bind (UDP_socket, (struct sockaddr*) &sock, sizeof(sock));

    if(error == -1){
        printf("Not made");
    }
    
    else{
        printf("\nConnection made \n");
    }
    return UDP_socket;

}


//RECEIVE MESSAGE: Receives the message and adds it to the list for printing later
void recv_msg(void* arguments){
    // printf("IN RECEIVE\n");
    struct recv_args* vals = (struct recv_args*) arguments;
    int receiver_socket2 = vals->sock;
    List* the_list = vals->a_list;
    int myPort = vals->myP;

    struct sockaddr_in sender_info;
    socklen_t sender_info_size = sizeof sender_info;


    while(1){

        char* message = (char*) malloc(MAX_LEN*sizeof(char));
        //Receiving and storing it in message array
        int msg_bytes = recvfrom(receiver_socket2, 
                    message, 
                    MAX_LEN, 
                    //sizeof(message),
                    0, 
                    (struct sockaddr *) &sender_info, 
                    &sender_info_size);

        if (msg_bytes == -1){
            printf("\nError with receiving\n");
        }

        List_append(the_list, message);

        //Signal OKAY TO PRINT
        pthread_mutex_lock(&ok_to_print_mutex);
        {
            pthread_cond_signal(&ok_to_print);
        }
        pthread_mutex_unlock(&ok_to_print_mutex);
    }

}


//PRINTS MESSAGE: Prints the message and removes from list
void print_msg(void* arguments){

    while(1){
        struct recv_args* values = (struct recv_args*) arguments;
        List* the_list = values->a_list;
        int myPort = values->myP;

        //Waits for signal that it is okay to print
        pthread_mutex_lock(&ok_to_print_mutex);
        {
            pthread_cond_wait(&ok_to_print, &ok_to_print_mutex);
        }
        pthread_mutex_unlock(&ok_to_print_mutex);

        printf("%s",(char *)List_curr(the_list));
        List_remove(the_list);
    }

}


//FGETS FUNCTION: Adds user typed message and adds to the list 
//Will be used on the main thread
void keyboard_input(void* s_args){

    struct send_args* vals2 = (struct send_args*) s_args;
    int sender_socket = vals2->sock2;
    List* the_list = vals2->a_list;
    char* IP_address = vals2->IP_add;
    unsigned short port_address = vals2->port_add;

    while(1){
        char* msg = (char*) malloc(MAX_LEN*sizeof(char));
        fgets( msg, MAX_LEN-1, stdin);

        //Terminates program
        if( *msg == '!'){
            exit(0);
            //close(socket);
        }
        List_append(the_list, msg);

        //SIGNAL OK TO SEND
        pthread_mutex_lock(&ok_to_send_mutex);
        {
            pthread_cond_signal(&ok_to_send);
        }
        pthread_mutex_unlock(&ok_to_send_mutex);
    }
}


//SEND MESSAGE: Sends it and removes from list
void send_msg(void* arguments2){

    struct send_args* vals2 = (struct send_args*) arguments2;
    int sender_socket = vals2->sock2;
    List* the_list = vals2->a_list;
    char* IP_address = vals2->IP_add;
    unsigned short port_address = vals2->port_add;

    while(1){


        //Waits for signal that it is okay to send
        pthread_mutex_lock(&ok_to_send_mutex);
        {
            pthread_cond_wait(&ok_to_send, &ok_to_send_mutex);
        }
        pthread_mutex_unlock(&ok_to_send_mutex);

        struct sockaddr_in receiver;
        receiver.sin_addr.s_addr = inet_addr(IP_address);
        receiver.sin_port = htons(port_address);

        int error = sendto(sender_socket, 
                    the_list->pCurrentNode->pItem, 
                    MAX_LEN, 
                    0, 
                    (struct sockaddr*) &receiver, 
                    sizeof(receiver));
        
        if (error == -1){
            printf("\nError\n");
            return;
        }

    List_remove(the_list);
    }
}


//This thread is used for printing the message that is received and removing it from the list
void prints_thread(struct recv_args* r_args){
    pthread_create(
        &print_thread,
        NULL,
        (void*) print_msg,
        r_args
    );
}


//This thread is for receiving the message and adding it to the shared list
void recv_thread(struct recv_args* r_args){
    pthread_create(
        &rcv_thread,
        NULL,
        recv_msg,
        r_args
    );
}

//This thread is for sending the mesasge and removing it from the list
void sending_thread(struct send_args* s_args){
    pthread_create(
        &send_thread,
        NULL,
        send_msg,
        s_args
    );
}
