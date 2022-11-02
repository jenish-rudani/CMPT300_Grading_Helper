#ifndef _S_TALK_H_
#define _S_TALK_H_

#define MAX_DATA_SIZE 100

// Take the arguments passed through main and initialize its corresponding variables.
void initializeArguements(char* arg1, char* arg2, char* arg3);

// Initialize the connection by calling checkConnection.
// Initialize the threads for all the processes.
void startCommunication();

// Initialize a socket and bind it to the specified internet address.
int createListenerConnection(struct addrinfo hints);

// Initialize a socket to sent data through. 
int createTalkerConnection(struct addrinfo hints);

// Send out an initial exchange of packets with the specificed remote port.
// If communication (send and recieve) is successful, return 1.
int checkConnection();

// When an "!" is received, this function gets called and sends it back to the other client.
// After, connectionRunning is set to 0, ending the while loop of all threads. 
void closeConnection();

// The UDP input thread, on receipt of input from the remote s-talk client, will put the 
// message onto the list of messages that need to be printed to the local screen.
void* receiveData(void* unused);

// The screen output thread will take each message off this list and output it to the screen.
void* receiveInput(void* unused);

// The keyboard input thread, on receipt of input, adds the input to the list of messages 
// that need to be sent to the remote s-talk client. 
void* sendOutput(void* unused);

// The UDP output thread will take each message off this list and send it over the network 
// to the remote client.
void* sendData(void* unused);


#endif