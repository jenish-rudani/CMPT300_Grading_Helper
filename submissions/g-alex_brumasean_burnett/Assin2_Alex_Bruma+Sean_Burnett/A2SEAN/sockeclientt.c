#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "listSock.h"



#define MAX 100

int myport;
int sendport;
static int socketDescriptor;

char messageRx[MAX];
char messageSx[MAX];
bool BREAK = false;
bool recv1 = false;
bool send1 = false;

int signal = 0;


//1
void* kbdrcvm(void* ptr){

    while(1){
        if(fgets(messageRx, MAX, stdin)){
            pbuffI();
            pI();
            pushtoInputList((void*) messageRx);
            vI();
            vitemI();
            if(messageRx[0] == '!'|| BREAK){
                BREAK = true;
                send1 = true;
                breaks();
                break;
            }
            
        }
    }

}
// %[^\n]
void* sendm(void* ptr){
    while(1){
        struct sockaddr_in sinRemote;
        sinRemote.sin_family = AF_INET;
        sinRemote.sin_addr.s_addr = INADDR_ANY;
        sinRemote.sin_port = htons(sendport);
        unsigned int sin_len = sizeof(sinRemote);
        sin_len = sizeof(sinRemote);
        pitemI();
        if(BREAK && recv1){
            break;
        } 
        pI();
        char* msg = (char*)popInputList();
        vI();
        vbuffI();
        sendto(socketDescriptor, msg, strlen(msg), 0 , (struct sockaddr*) &sinRemote, sin_len);
        if(BREAK){
            break;
        } 
        
    }

}
//3
void* recvm(void* ptr){
    while(1){
        
        if(BREAK){
            break;
        }
        
        struct sockaddr_in sinRemote;
        sinRemote.sin_family = AF_INET;
        sinRemote.sin_addr.s_addr = INADDR_ANY;
        sinRemote.sin_port = htons(sendport);
        unsigned int sin_len = sizeof(sinRemote);
        sin_len = sizeof(sinRemote);
        
        recvfrom(socketDescriptor, messageSx, MAX, 0, (struct sockaddr *) &sinRemote, &sin_len);
        pbuffO();
        pO();
        pushtoOutputList((void*)messageSx);
        vO();
        vitemO();
        if(messageSx[0] == '!' || BREAK){
            BREAK = true;
            recv1 = true;
            struct sockaddr_in sinRemote;
            sinRemote.sin_family = AF_INET;
            sinRemote.sin_addr.s_addr = INADDR_ANY;
            sinRemote.sin_port = htons(sendport);
            unsigned int sin_len = sizeof(sinRemote);
            sin_len = sizeof(sinRemote);

            sendto(socketDescriptor, messageSx, strlen(messageSx), 0 , (struct sockaddr*) &sinRemote, sin_len);
            messageRx[0] = '1';
            breaks();
            break;
            
        }
    }
}
//4
void* prntscrn(void* ptr){
    while(1){
        //printf("printcscr");
        pitemO();
        if(BREAK && send1){
            break;
        }
        pO();
        char* msg = (char*)popOutputList();
        fputs(msg, stdout);
        memset(messageSx, 0, MAX);
        vO();
        vbuffO();
        if(BREAK){
            break;
        }
            
    }
}


int main(){ 

    char buffer[100];
    scanf("%[^\n]",buffer);
   // printf("%s", buffer);
    char m1[30], thisport[30], machinename[30], sendtoport[30];
    int total_read;

    total_read = sscanf(buffer, "%s %s %s %s" ,m1, thisport,machinename, sendtoport);

    myport = atoi(thisport);
    sendport = atoi(sendtoport);
    // printf("\nTotal items read: %d",total_read);


    List_init();
    

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(myport);
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));


    pthread_t thread0, thread1, thread2, thread3;

    int iret0 = pthread_create(&thread0, NULL, kbdrcvm, NULL);
    int iret1 = pthread_create(&thread1, NULL, sendm, NULL);
    int iret2 = pthread_create(&thread2, NULL, recvm, NULL);
    int iret3 = pthread_create(&thread3, NULL, prntscrn, NULL);



    
    pthread_join( thread0, NULL);
    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);
    pthread_join( thread3, NULL);


    return 0;
}