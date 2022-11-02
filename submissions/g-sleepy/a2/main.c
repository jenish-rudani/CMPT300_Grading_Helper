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


//Threads
extern pthread_t fget_thread, rcv_thread, print_thread, send_thread;

//MUTEX/CONDITIONS

//Goes into receive function, signalling that it is ok to print
extern pthread_cond_t ok_to_print;
extern pthread_mutex_t ok_to_print_mutex;

//Goes into keyboard function, signalling that it is okay to send
extern pthread_cond_t ok_to_send;
extern pthread_mutex_t ok_to_send_mutex;

int main(int argc, char *argv[]){

    //Shared List
    List* shared_list = List_create();

    //s-talk [my port number] [remote machine name] [remote port number]
    char* IP_address;
    int my_port;
    int remote_port;

    if(argc == 4){
        //IP ADRESS
        struct hostent *IP_num = gethostbyname(argv[2]);
        if(IP_num==NULL){
            printf("Error getting IP address from machine name");
        }
        IP_address = inet_ntoa(*((struct in_addr*) IP_num->h_addr_list[0]));

        //PORT ADDRESSES
        my_port = atoi(argv[1]);
        remote_port = atoi(argv[3]);
        printf("IP address: %s, My Port Address: %d, Remote Port Address %d\n", IP_address, my_port, remote_port);
    }

    else{
        printf("Please put in correct arguments\n");
        return 0;
    }

    //Connect to remote port
    int U_socket = create_connection(IP_address, remote_port);

    //The multithreading
    struct send_args* s_argument = malloc(sizeof(struct send_args));
    s_argument->a_list = shared_list;
    s_argument->IP_add = IP_address;
    s_argument->port_add = my_port;
    s_argument->sock2 = U_socket;
    s_argument -> sturn = 0;


    struct recv_args* r_argument = malloc(sizeof(struct recv_args));
    r_argument->sock = U_socket;
    r_argument->a_list = shared_list;
    r_argument->myP = my_port;
    r_argument-> rturn = 1;


    // //Thread 2: Sends
    sending_thread(s_argument);

    //Thread 3: Receives
    recv_thread(r_argument);

    // //CURRENT PROBLEM, HOW DO I KNOW TO PRINT OR TO CONTINUE
    prints_thread(r_argument);

    // //Thread 1 (Main Thread): Gets keyboard input
    keyboard_input(s_argument);

    //Cleanup
    (void) pthread_join(send_thread, NULL);
    (void) pthread_join(rcv_thread, NULL);
    (void) pthread_join(print_thread, NULL);

}