//$hostname

#include <stdio.h>
#include <stdlib.h>
#include<string.h> //memset
#include<sys/socket.h>
#include<errno.h> //For errno - the error number
#include<netdb.h>	//hostent
#include<arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

#include <curses.h>
#include <pthread.h>
#include "utils.h"
#include "list.h"

List mylist,mylistrx;

void *kb_th_func(void *arg) {
	//char buf[1024];
	while(1){

		
		ch = getch();
		
		char *chp=(char*)&ch;
		pthread_mutex_lock(&lock);
			insertback(&mylist,chp);
			done = 1 ;
			pthread_cond_signal(&cond);
		pthread_mutex_unlock(&lock) ;
	}
}

void *udp_in_th_func(void *arg) {

	char sendbuf[MAXSIZE];
	socklen_t len=sizeof(theiraddr);
	int n;
	while(1){

		n = recvfrom(sockfd, (char *)sendbuf, sizeof(sendbuf),  
			MSG_WAITALL, ( struct sockaddr *) &theiraddr, 
			&len); 
		sendbuf[n] = '\0'; 

		
		pthread_mutex_lock(&lockrx);
			insertback(&mylistrx,sendbuf);
			donerx = 1 ;
			pthread_cond_signal(&condrx);
		pthread_mutex_unlock(&lockrx) ;	
		
	}
}

void *screen_out_th_func(void *arg) {

	char sendbuf[MAXSIZE];
	//int len=sizeof(theiraddr);
	
	while(1){
		
		pthread_mutex_lock(&lockrx) ;
			while( donerx == 0 )
			pthread_cond_wait(&condrx ,&lockrx) ; // call the function inside the thr_join () function.
			donerx=0;
			memcpy(sendbuf, mylistrx.front->data,sizeof(mylistrx.front->data));
			//printf("SendBuf::%s\n",sendbuf);
			removeFromList(&mylistrx);
			
		pthread_mutex_unlock(&lockrx) ;
		

		printf("%s\n",sendbuf);
		
		if(strncmp(sendbuf,"!",1)==0){
			printf("I am exitting\n");
			close(sockfd);
			destroy(&mylist);
			destroy(&mylistrx);
			
			refresh();			
			getch();			
			endwin();			

			exit(0);
		}	
		
	}
}

void *udp_out_th_func(void *arg) {

	char sendbuf[MAXSIZE];
	int len=sizeof(theiraddr);
	
	while(1){
		
		pthread_mutex_lock(&lock) ;
			while( done == 0 )
			pthread_cond_wait(&cond ,&lock) ; // call the function inside the thr_join () function.
			done=0;
			memcpy(sendbuf, mylist.front->data,sizeof(mylist.front->data));
			//printf("SendBuf::%s\n",sendbuf);
			removeFromList(&mylist);
			
		pthread_mutex_unlock(&lock) ;
		
		sendto(sockfd, (const char *)sendbuf, sizeof(sendbuf),  
			MSG_CONFIRM, (const struct sockaddr *) &theiraddr, 
			len);
			
		if(strncmp(sendbuf,"!",1)==0){
			printf("I am exitting\n");
			close(sockfd);
			destroy(&mylist);
			destroy(&mylistrx);
			
			refresh();			
			getch();			
			endwin();			


			exit(0);
		}

	}
}


int main(int argc, char *argv[]){

	
	printf("Hello\n");
	done=0;
	donerx=0;
	initlist(&mylist);
	initlist(&mylistrx);
	
	
	
	if(argc!=4){
		printf("Wrong Number of Arguments; [my port number] [remote machine name] [remote port number]\n");exit(EXIT_FAILURE);
	}
	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("\n mutex init has failed\n");
		exit(EXIT_FAILURE);
	}
	
	if (pthread_mutex_init(&lockrx, NULL) != 0) {
		printf("\n mutex init has failed\n");
		exit(EXIT_FAILURE);
	}
	pthread_cond_init(&cond,NULL);
	pthread_cond_init(&condrx,NULL);
	//cond = PTHREAD_COND_INITIALIZER;
	///Creating threads
	pthread_t kb_th,udp_out_th,udp_in_th,screen_out_th;
//	void *retth;

	List mylist;
	initlist(&mylist);




	char *hostname = argv[2];
	int myport=(atoi)(argv[1]);
	int theirport=(atoi)(argv[3]);
	char ip[100];


	
	
	hostname_to_ip(hostname , ip);
	printf("%s resolved to %s\n" , hostname , ip);
	
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&theiraddr, 0, sizeof(theiraddr)); 
	memset(&myaddr, 0, sizeof(myaddr)); 

	// Filling my information 
	myaddr.sin_family    = AF_INET; // IPv4 
	myaddr.sin_addr.s_addr = INADDR_ANY; 
	myaddr.sin_port = htons(myport); 	
	
	theiraddr.sin_family    = AF_INET; // IPv4
	inet_pton(AF_INET, ip, &(theiraddr.sin_addr)); 
	theiraddr.sin_port = htons(theirport); 	


	// Bind the socket with the server address 
	if ( bind(sockfd, (const struct sockaddr *)&myaddr,  
	    sizeof(myaddr)) < 0 ) 
	{
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}
	
	initscr();
	cbreak();			
	keypad(stdscr, TRUE);		
	noecho();			
	
	
	
	if (pthread_create(&kb_th, NULL, kb_th_func, NULL) != 0) {
		perror("pthread_create() kb_th_func error");
		exit(EXIT_FAILURE);
	}

	if (pthread_create(&udp_out_th, NULL, udp_out_th_func, NULL) != 0) {
		perror("pthread_create() udp_out_th error");
		exit(EXIT_FAILURE);
	}
	
	if (pthread_create(&udp_in_th, NULL, udp_in_th_func, NULL) != 0) {
		perror("pthread_create() udp_in_th error");
		exit(EXIT_FAILURE);
	}
	
	if (pthread_create(&screen_out_th, NULL, screen_out_th_func, NULL) != 0) {
		perror("pthread_create() udp_in_th error");
		exit(EXIT_FAILURE);
	}
	
	if (pthread_join(udp_in_th, NULL) != 0) {
		perror("pthread_create() error");
		exit(EXIT_FAILURE);
	}
	
	if (pthread_join(kb_th, NULL) != 0) {
		perror("pthread_create() error");
		exit(EXIT_FAILURE);
	}
	
	if (pthread_join(udp_out_th, NULL) != 0) {
		perror("pthread_create() error");
		exit(EXIT_FAILURE);
	}
	
	if (pthread_join(screen_out_th, NULL) != 0) {
		perror("pthread_create() error");
		exit(EXIT_FAILURE);
	}
	
}


