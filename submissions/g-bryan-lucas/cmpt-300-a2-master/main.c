// Group Name: Bryan & Lucas
// Lucas Mah 301422499
// Bryan Dang 301432330

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "list.h"

#define BUFFER_SIZE 256

static pthread_mutex_t send_mutex;
static pthread_mutex_t recv_mutex;

static pthread_cond_t send_count_nonzero;
static pthread_cond_t recv_count_nonzero;

static pthread_t kb_input_thread;
static pthread_t UDP_output_thread;
static pthread_t UDP_input_thread;
static pthread_t screen_output_thread;

struct ThreadData {
    int socket;
    struct sockaddr_in remote_addr;
    List* list;
    int *terminate;
};

struct InputData {
    int port_num;
    int remote_port_num;
    char remote_machine_name[20];
};

void* kb_input(void* send_list) {
    char buffer[BUFFER_SIZE];
    char* export;

    while (1) {
        // User input for message
        fgets(buffer, BUFFER_SIZE, stdin);

        // Attach message to send_list if it's not blank
        if (strcmp(buffer, "\n") != 0) {
            export = (char*) malloc(sizeof(char) * BUFFER_SIZE);
            strcpy(export, buffer);
            assert(pthread_mutex_lock(&send_mutex) == 0);
            
            if (List_count(send_list) == 0) {
                List_prepend(send_list, (char*)export);
                pthread_cond_signal(&send_count_nonzero);
            }
            
            assert(pthread_mutex_unlock(&send_mutex) == 0);

            // terminate thread if '!' is typed 
            if (strcmp(export, "!\n") == 0) {
                break;
            }
        }
    }

    return NULL;
}

void* UDP_output(void* data) {
    // Convert void* to ThreadData*
    struct ThreadData *sub = data;

    // Extract data from struct
    int socket = sub->socket;
    struct sockaddr_in addr = sub->remote_addr;
    List* send_list = sub->list;
    int *terminate = sub->terminate;

    printf("Remote address:\t%d, %d\n\n", addr.sin_port, addr.sin_addr.s_addr);
    
    // Check if valid remote address - terminate program if not
    if (addr.sin_port == 0) {
        printf("Invalid input... program terminating\n");
        exit(0);
    } else {
        printf("Connection success! Say something:\n");
    }

    // Free struct
    free(data);

    char* send_msg;
    int signal = 0;

    while(*terminate == 0) {
        // sends message when the list is not empty and is woken up 
        assert(pthread_mutex_lock(&send_mutex) == 0);

        // break out of loop if terminate is set to 1 or list is no longer empty
        while (List_count(send_list) == 0 && *terminate == 0)
            pthread_cond_wait(&send_count_nonzero, &send_mutex);

        // only send message if the list is not empty
        if (List_count(send_list) != 0) {
            send_msg = (char*)List_last(send_list);
            signal = sendto(socket, send_msg, BUFFER_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr));
            (signal > 0) ? List_trim(send_list) : perror("Error: ");
        }

        assert(pthread_mutex_unlock(&send_mutex) == 0);

        // properly free buffers depending on who terminated program
        if (strcmp(send_msg, "!\n") == 0) {
            *terminate = 1;
            free(send_msg);
            pthread_cancel(UDP_input_thread);
            pthread_cancel(screen_output_thread);
        } else if (*terminate == 0) {
            free(send_msg);
        }
    }
    return NULL;
}

void* UDP_input(void* data) {
    // Convert void* to ThreadData*
    struct ThreadData *sub = data;

    // Extract data from struct
    int socket = sub->socket;
    struct sockaddr_in addr = sub->remote_addr;
    List* recv_list = sub->list;
    int *terminate = sub->terminate;
    socklen_t len = sizeof(addr);
    
    // Free struct
    free(data);

    int signal = 0;
    char recv_msg[BUFFER_SIZE];
    char* export;

    while(1) {
        signal = recvfrom(socket, recv_msg, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&addr, &len);
        if (signal < 0) perror("Error: ");

        export = (char*) malloc(sizeof(char) * BUFFER_SIZE);
        strcpy(export, recv_msg);

        // if '!' was sent, set terminate to 1 and wake up UDP_output thread to terminate program
        if (strcmp(export, "!\n") == 0) {
            pthread_cancel(kb_input_thread);
            pthread_cancel(UDP_output_thread);
            pthread_cancel(screen_output_thread);
            free(export);
            break;
        }

        pthread_mutex_lock(&recv_mutex);
        
        // add non empty messages to list to output onto screen
        if (List_count(recv_list) == 0) {
            List_prepend(recv_list, (char*)export);
            pthread_cond_signal(&recv_count_nonzero);
        }

        pthread_mutex_unlock(&recv_mutex);
    }

    // terminating program procedures
    *terminate = 1;
    pthread_cond_signal(&send_count_nonzero);
    return NULL;
}

void* screen_output(void* recv_list) {
    char *output_msg;
    while(1) {
        pthread_mutex_lock(&recv_mutex);

        while (List_count(recv_list) == 0)
            pthread_cond_wait(&recv_count_nonzero, &recv_mutex);

        output_msg = List_trim(recv_list);

        pthread_mutex_unlock(&recv_mutex);

        printf("[connection]: %s", output_msg);
        free(output_msg);
    }
    return NULL;
}

void menu() {
    printf("Welcome to s-talk!\n");
    printf("s-talk [port number] [remote machine name] [port number]\n");
    printf("--------------------------------------------------------\n");
}

struct InputData init_s_talk() {
    char user_input[BUFFER_SIZE];
    struct InputData input;
    
    // Get user input and extract data
    printf("Input: ");
    fgets(user_input, sizeof user_input, stdin);

    char *token = strtok(user_input, " ");
    int counter = 0;

    while (token != NULL) {
        switch(counter) {
            case 0:
                break;
            case 1:
                input.port_num = atoi(token);
                break;
            case 2:
                strcpy(input.remote_machine_name, token);
                break;
            case 3:
                input.remote_port_num = atoi(token);
                break;
            default:
                printf("Error! More parameters than expected.\n");
                break;
        }

        token = strtok(NULL, " ");
        counter++;
    }
    return input;
}

int main() {
    menu();
    struct InputData input = init_s_talk(&input);
    
    pthread_mutex_init(&send_mutex, NULL);
    pthread_mutex_init(&recv_mutex, NULL);
    pthread_cond_init(&send_count_nonzero, NULL);
    pthread_cond_init(&recv_count_nonzero, NULL);

    // Initalize socket
    int s;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == -1) {
        printf("socket failed\n");
    }

    // Address for local machine
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(input.port_num);
    addr.sin_addr.s_addr = INADDR_ANY;
    memset(&addr.sin_zero, '\0', 8);

    bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    printf("My address:\t%d\n", addr.sin_port);

    // Address for remote machine
    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    struct hostent *hp;
    hp = gethostbyname(input.remote_machine_name);

    if (hp) {
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_port = htons(input.remote_port_num);
        remote_addr.sin_addr.s_addr = *((unsigned long *)hp->h_addr);
        memset(&remote_addr.sin_zero, '\0', 8);
    }

    // Setup parameters for threads
    List* send_list = List_create();
    List* recv_list = List_create(); 
    int end = 0;

    struct ThreadData *udp_out;
    udp_out = malloc(sizeof(struct ThreadData));
    udp_out->socket = s;
    udp_out->remote_addr = remote_addr;
    udp_out->list = send_list;
    udp_out->terminate = &end;

    struct ThreadData *udp_in;
    udp_in = malloc(sizeof(struct ThreadData));
    udp_in->socket = s;
    udp_in->remote_addr = remote_addr;
    udp_in->list = recv_list;
    udp_in->terminate = &end;

    pthread_create(&kb_input_thread, NULL, kb_input, (void*) send_list);
    pthread_create(&UDP_output_thread, NULL, UDP_output, (void*) udp_out);
    pthread_create(&UDP_input_thread, NULL, UDP_input, (void*) udp_in);
    pthread_create(&screen_output_thread, NULL, screen_output, (void*) recv_list);
    
    // Wait for UDP_output thread for termination
    pthread_join(kb_input_thread, NULL);         
    pthread_join(UDP_output_thread, NULL);
    pthread_join(UDP_input_thread, NULL);       
    pthread_join(screen_output_thread, NULL);   

    pthread_mutex_destroy(&send_mutex);
    pthread_mutex_destroy(&recv_mutex);

    // Destroy condition variables - set wrefs to 0 to avoid hang
    send_count_nonzero.__data.__wrefs = 0;
    pthread_cond_destroy(&send_count_nonzero);
    recv_count_nonzero.__data.__wrefs = 0;
    pthread_cond_destroy(&recv_count_nonzero);

    // Close socket
    close(s);

    printf("s-talk session terminated.\n");
}
