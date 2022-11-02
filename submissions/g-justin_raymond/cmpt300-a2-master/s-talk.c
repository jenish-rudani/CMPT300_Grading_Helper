#include "includes/list.h"
#include "includes/socketIO.h"
#include "includes/userIO.h"

// Initialize all utilities needed for functionality
List* keyboardList;
List* screenList;

int rcvSocket;
int sendSocket;

char *myPortNumber;
char *remotePortNumber;
char *remoteMachineName;
bool endFlag = false;

pthread_mutex_t screenListMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t keyboardListMutex = PTHREAD_MUTEX_INITIALIZER;

// Needed for freeing the lists
void freeFunction(void* pItem){
    free(pItem);
};

int main(int argc, char **argv){
    if (argc < 2){
        printf("Usage: %s <my port number> <remote machine name> <remote port number>\n", argv[0]);
        exit(-1);
    }

    keyboardList = List_create();
    screenList = List_create();
    // Extract inputs into legible variables
    myPortNumber = argv[1];
    remoteMachineName = argv[2];
    remotePortNumber = argv[3];
    
    // Initialize all input structs needed for each thread
    socketIN recvIN = {screenList, &rcvSocket, &myPortNumber, NULL, NULL, &endFlag, &screenListMutex};
    socketIN sendIN = {keyboardList, &sendSocket, NULL, &remotePortNumber, &remoteMachineName, &endFlag, &keyboardListMutex};
    userIO_IN keyboardIN = {keyboardList, &keyboardListMutex, &endFlag};
    userIO_IN screenIN = {screenList, &screenListMutex, &endFlag};

    // Create all threads
    pthread_t keyboard, screen, send, recv;
    if (pthread_create(&recv, NULL, &UDP_recv, &recvIN) < 0){
        perror("Could not create recv thread\n");
        return 1;
    }
    if (pthread_create(&send, NULL, &UDP_send, &sendIN) < 0){
        perror("Could not create send thread\n");
        return 1;
    }
    if (pthread_create(&keyboard, NULL, &keyboardRead, &keyboardIN) < 0){
        perror("Could not create keyboardRead thread\n");
        return 1;
    }
    if (pthread_create(&screen, NULL, &screenWrite, &screenIN) < 0){
        perror("Could not create screenWrite thread\n");
        return 1;
    }

    // While all other threads are running, the main will busy-wait, checking when to end the session.
    while(!endFlag);

    pthread_cancel(recv);
    pthread_cancel(send);
    pthread_cancel(keyboard);
    pthread_cancel(screen);
    close(rcvSocket);
    close(sendSocket);
    List_free(keyboardList, &freeFunction);
    List_free(screenList, &freeFunction);

    printf("Exiting s-talk session...\n");
    
    return 0;
}
