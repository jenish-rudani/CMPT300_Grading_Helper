#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "list.h"

#define BUFFER_SIZE 2048

// gloabally needed varibles
int terminated;
char remoteHostName[256];
char portNum[256];
char remotePort[256];

List *input;
List *recieved;

// mutex locks
pthread_mutex_t input_sender_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t receiver_output_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t input_sender_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t receiver_output_cond = PTHREAD_COND_INITIALIZER;

// sends data to the other side
// creates the necessary socket and sends the data
// whenever it is available in the
void *sender()
{

    // getting server address
    puts("getting server address...");
    struct hostent *server = gethostbyname(remoteHostName);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR: no such host '%s'\n",remoteHostName);
        exit(1);
    }

    // Creating socket file descriptor
    puts("creating client socket...");
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "Could not create client socket\n");
        exit(1);
    }

    // Filling server information
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    // get the port
    puts("Ready to send!");
    int PORT = atoi(remotePort);
    if (PORT <= 0)
    {
        fprintf(stderr, "ERROR: invalid port number '%s'\n", remotePort);
        exit(1);
    }

    // Filling server information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;


    while (!terminated)
    {

        // lock the input list
        pthread_mutex_lock(&input_sender_lock);

        // wait for input
        while (input->size == 0 && !terminated)
        {
            pthread_cond_wait(&input_sender_cond, &input_sender_lock);
        }
        if (terminated)
        {
            pthread_mutex_unlock(&input_sender_lock);
            break;
        }

        // get the first item in the list
        char *data = (char *)List_trim(input);

        // unlock the input list
        pthread_mutex_unlock(&input_sender_lock);

        // send the data
        sendto(sockfd, (const char *)data, strlen(data),
               MSG_CONFIRM, (const struct sockaddr *)&server_addr,
               sizeof(server_addr));

        // free the data
        free(data);
    }
    close(sockfd);
    return 0;
}

// creates the recieving end
// accepts data from the other side
void *server()
{

    // Creating socket file descriptor
    puts("creating server socket...");
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "Could not create server socket\n");
        exit(1);
    }

    // Filling server information
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    // get the port
    puts("Ready to recieve!");
    int PORT = atoi(portNum);
    if (PORT <= 0)
    {
        fprintf(stderr, "ERROR: invalid port number '%s'\n", portNum);
        exit(1);
    }

    // Filling server information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "Could not bind client socket\n");
        exit(1);
    }

    unsigned n, len;

    while (!terminated)
    {

        // get the data
        char buffer[BUFFER_SIZE];
        n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE,
                     MSG_WAITALL, (struct sockaddr *)&server_addr,
                     &len);
        buffer[n] = '\0';

        // lock the output list
        pthread_mutex_lock(&receiver_output_lock);

        // add the data to the output list
        char *data = strdup(buffer);

        while(List_prepend(recieved,data)){ //wait until the data is added to the list
            pthread_cond_wait(&receiver_output_cond, &receiver_output_lock);
        }

        // signal the output thread to print the data
        pthread_cond_signal(&receiver_output_cond);

        // unlock the output list
        pthread_mutex_unlock(&receiver_output_lock);
    }
    close(sockfd);
    return 0;
}


void *handle_input(){


    puts("");
    while(!terminated){
            
            // get the data
            char buffer[BUFFER_SIZE];
           
            fgets(buffer, BUFFER_SIZE, stdin);
    
            // lock the input list
            pthread_mutex_lock(&input_sender_lock);
    
            // add the data to the input list
            char *data = strdup(buffer);
    
            while(List_append(input,data)){ //wait until the data is added to the list
                pthread_cond_wait(&input_sender_cond, &input_sender_lock);
            }
    
            // signal the sender thread to send the data
            //signal that data is now available if it was waiting as the list size was
            // 0
            pthread_cond_signal(&input_sender_cond);
    
            // unlock the input list
            pthread_mutex_unlock(&input_sender_lock);
    }
    return NULL;
}

void *handle_output(){

    puts("");
    while(!terminated){
            
            // lock the output list
            pthread_mutex_lock(&receiver_output_lock);
    
            // wait for output
            while (recieved->size == 0 && !terminated)
            {
                pthread_cond_wait(&receiver_output_cond, &receiver_output_lock);
            }
            if (terminated)
            {
                pthread_mutex_unlock(&receiver_output_lock);
                break;
            }
    
            // get the first item in the list
            char *data = (char *)List_trim(recieved);
    
            // unlock the output list
            pthread_mutex_unlock(&receiver_output_lock);
    
            // print the data
            printf("%s:%s>> %s\n", remoteHostName,remotePort,data);
    
            // free the data
            free(data);
    }

    return NULL;
}

int main(int argc, char *argv[])
{

    // check the arguments
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <port> <remote_host_name> <remote_port>\n", argv[0]);
        exit(1);
    }

    strcpy(portNum, argv[1]);
    strcpy(remoteHostName, argv[2]);
    strcpy(remotePort, argv[3]);

    // create the input list
    input = List_create();
    recieved = List_create();

    // create the threads
    pthread_t sender_thread;
    pthread_t receiver_thread;
    pthread_t input_thread;
    pthread_t output_thread;

    // create the sender thread
    if (pthread_create(&sender_thread, NULL, sender, NULL) != 0)
    {
        fprintf(stderr, "Could not create sender thread\n");
        exit(1);
    }

    // create the receiver thread
    if (pthread_create(&receiver_thread, NULL, server, NULL) != 0)
    {
        fprintf(stderr, "Could not create receiver thread\n");
        exit(1);
    }

    // create the input thread
    if (pthread_create(&input_thread, NULL, handle_input, NULL) != 0)
    {
        fprintf(stderr, "Could not create input thread\n");
        exit(1);
    }

    // create the output thread
    if (pthread_create(&output_thread, NULL, handle_output, NULL) != 0)
    {
        fprintf(stderr, "Could not create output thread\n");
        exit(1);
    }

    // wait for the threads to finish
    pthread_join(sender_thread, NULL);
    pthread_join(receiver_thread, NULL);
    pthread_join(input_thread, NULL);
    pthread_join(output_thread, NULL);

    // free the list
    List_free(input,free);
    List_free(recieved, free);

    return 0;
}