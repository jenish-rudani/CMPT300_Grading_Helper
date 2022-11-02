/*Parm Gidda - 301423241 - psg14 - psg14@sfu.ca
 Learned and adpated how to use threads, sockets, and condition variables from videos provied by 
 Brian Fraser as well as documents provided in assignment 
 Learned and adapted how to use sockets from https://www.geeksforgeeks.org/socket-programming-cc/
 Learend and adapted how to use getaddrinfo function from https://stackoverflow.com/questions/42138233/getaddrinfo-returns-previous-fixed-ip-address user "ZulKaz"
 and https://linuxhint.com/c-getaddrinfo-function-usage/
*/

#include <stdio.h>
#include "list.h"
#include "threads.h"
#include <stdio.h>

#include <pthread.h>

int main(){
	
	pthread_t mainThread;
	pthread_create(&mainThread, NULL, initThreads, NULL);
	pthread_join(mainThread, NULL);
 	pthread_cancel(mainThread);

}

