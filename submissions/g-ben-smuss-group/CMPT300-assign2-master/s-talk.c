#include "list.h"
#include <arpa/inet.h> // sockaddr, socket, bind, recvfrom, sendto, various constants
#include <assert.h> // assert
#include <netdb.h>  // getaddrinfo, gai_strerror
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>  // printf, sprintf
#include <stdlib.h> // atoi
#include <string.h> // memset, strlen
#include <unistd.h> // close
#include <termios.h>

#define MAX_LEN 1024
socklen_t sockaddr_length = sizeof(struct sockaddr);

List *list_rx, *list_tx;

pthread_mutex_t mutex_for_list_rx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_for_list_tx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_for_message_tx = PTHREAD_MUTEX_INITIALIZER;

int sfd_local, sfd_remote;

sem_t list_rx_produce_sem, list_rx_consume_sem, list_tx_produce_sem,
	list_tx_consume_sem, exit_sem;

pthread_t list_rx_produce_thread, list_rx_consume_thread,
	list_tx_produce_thread, list_tx_consume_thread;

char *message_rx, *message_tx;

char *port_local, *port_remote, *host_remote;

// the maximum number of elements that list_rx and list_tx can hold
const int MAX_ELEMS = LIST_MAX_NUM_NODES / 2;

struct termios tio_old, tio_new;

// restores terminal mode to canonical (buffered stdin)
void safe_exit(char *str)
{
	fprintf(stderr, "[crash -- %s]\n", str);
	tcsetattr(STDIN_FILENO, TCSANOW, &tio_old);
	exit(1);
}

int setup_local_socket(struct sockaddr_in *sin, int port_local) 
{
	memset(sin, 0, sockaddr_length);
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = htonl(INADDR_ANY);
	sin->sin_port = htons(port_local);
	int sfd = socket(PF_INET, SOCK_DGRAM, 0);

	if (sfd == -1) 
	{
		safe_exit("could not create socket");
	}

	if (bind(sfd, (struct sockaddr *)sin, sockaddr_length) == -1) 
	{
		safe_exit("could not bind socket");
	}

	return sfd;
}

// appends received messages to list_rx
void *list_rx_produce() 
{
	struct sockaddr_in sin;
	sfd_local = setup_local_socket(&sin, atoi(port_local));

	while (true) 
	{
		message_rx = malloc(MAX_LEN);

		int bytes_rx = recvfrom( 
			sfd_local, message_rx, MAX_LEN, 0,
			(struct sockaddr *)NULL, // struct gets filled in
			&sockaddr_length
		);

		if (bytes_rx == -1) 
		{
			safe_exit("'recvfrom' failed");
		}

		// null terminate message_rx
		int termination_index = (bytes_rx < MAX_LEN) ? bytes_rx : MAX_LEN - 1;
		message_rx[termination_index] = 0;

		sem_wait(&list_rx_produce_sem); // don't produce if full list

		pthread_mutex_lock(&mutex_for_list_rx);
		// printf("RX Producer in CS\n");
		List_append(list_rx, message_rx);
		pthread_mutex_unlock(&mutex_for_list_rx);

		sem_post(&list_rx_consume_sem); // enable consumer
	}
}

// removes messages from list_rx and prints them
void *list_rx_consume() 
{
	while (true) 
	{
		sem_wait(&list_rx_consume_sem); // don't consume if empty list

		pthread_mutex_lock(&mutex_for_list_rx);
		// printf("RX Consumer in CS\n");
		char *message_rx_trimmed = List_trim(list_rx);
		pthread_mutex_unlock(&mutex_for_list_rx);

		sem_post(&list_rx_produce_sem); // enable producer

		if (message_rx_trimmed[0] == '!') 
		{
			puts("['!' received: connection was closed]");
			sem_post(&exit_sem);
		} else
		{
			pthread_mutex_lock(&mutex_for_message_tx);
			if (message_tx[0] != '\0') {
				printf("\n--> %s", message_rx_trimmed);
				printf("[input resumed from '%s']\n", message_tx);
				printf("%s", message_tx);
				fflush(stdout);
			} else {
				// printf("message_tx (rx printer pov) = %s\n", message_tx);
				printf("--> %s", message_rx_trimmed);
			}
			pthread_mutex_unlock(&mutex_for_message_tx);
		}

		free(message_rx_trimmed);
	}
}

// reads lines from stdin and appends them to list_tx
void *list_tx_produce() 
{
	while (true) 
	{
		pthread_mutex_lock(&mutex_for_message_tx);
		message_tx = malloc(MAX_LEN);
		message_tx[0] = '\0';
		pthread_mutex_unlock(&mutex_for_message_tx);
		// fgets(message_tx, MAX_LEN, stdin);
		char ch = ' ';
		int i = 0;
		while (ch != '\n' && i < MAX_LEN) {
			ch = getchar();
			pthread_mutex_lock(&mutex_for_message_tx);
			if (ch == 8 || ch == 127) // backspace codes
			{ 
				if (i > 0) 
				{
					message_tx[--i] = '\0';
				}
			} else 
			{
				message_tx[i++] = ch;
			}
			printf("\33[2K\r");
			printf("%s", message_tx);
			fflush(stdout);
			pthread_mutex_unlock(&mutex_for_message_tx);
		}
		pthread_mutex_lock(&mutex_for_message_tx);
		message_tx[i] = '\0';
		pthread_mutex_unlock(&mutex_for_message_tx);

		sem_wait(&list_tx_produce_sem); // don't produce if full list

		pthread_mutex_lock(&mutex_for_list_tx);
		// printf("TX Producer in CS\n");
		List_append(list_tx, message_tx);
		pthread_mutex_unlock(&mutex_for_list_tx);

		sem_post(&list_tx_consume_sem); // enable consumer
	}
}

// takes message from list_tx and sends to sin_remote
void *list_tx_consume() 
{
	// Find the "struct sockaddr" based on host_remote and port_remote (which can
	// be used by sendto). "struct addrinfo" has a field ai_addr which is a
	// "struct sockaddr".
	struct addrinfo *ai_start = NULL;
	struct addrinfo *ai_curr = NULL;

	int error = getaddrinfo(host_remote, port_remote, NULL, &ai_start);

	if (error != 0) 
	{
		safe_exit((char *)gai_strerror(error));
	}

	printf("[trying to find IP/port...]");
	fflush(stdout);
	for (ai_curr = ai_start; ai_curr != NULL; ai_curr = ai_curr->ai_next)
	{
		sfd_remote = socket(ai_curr->ai_family, ai_curr->ai_socktype,
							ai_curr->ai_protocol);
		if (sfd_remote == -1) 
		{
			continue; // couldn't open file descriptor, move on.
		}
		int er = connect(sfd_remote, ai_curr->ai_addr, ai_curr->ai_addrlen);
		if (er == 0) 
		{
			puts("[IP/port found]");
			break; // successful connect
		}
		close(sfd_remote); // only happens if unsuccessful connect
	}


	if (ai_curr == NULL) 
	{
		safe_exit("could not connect to socket");
	} 

	while (true)
	{
		sem_wait(&list_tx_consume_sem); // don't consume if empty list

		pthread_mutex_lock(&mutex_for_list_tx);
		// printf("TX Consumer in CS\n");
		char *message_tx_trimmed = List_trim(list_tx);
		pthread_mutex_unlock(&mutex_for_list_tx);

		sem_post(&list_tx_produce_sem); // enable producer

		int chars_sent = sendto(sfd_remote, message_tx_trimmed, strlen(message_tx_trimmed), 0,
						ai_curr->ai_addr, ai_curr->ai_addrlen);

		if (chars_sent == -1) 
		{
			puts("[message could not be sent]");
		}

		if (message_tx_trimmed[0] == '!') 
		{
			puts("['!' sent: closing connection]");
			sem_post(&exit_sem);
		}

		free(message_tx_trimmed);
	}
}

int main(int argc, char **args) 
{
	if (argc != 4) {
		fprintf(stderr, "Invalid argument count\nusage: s-talk port_local host_remote port_remote\n");
		return 1;
	}

	port_local = args[1];
	host_remote = args[2];
	port_remote = args[3];

	message_rx = message_tx = "";

	// setting terminal to a different mode
	// not waiting for enter to read characters (for getchar)
	tcgetattr(STDIN_FILENO, &tio_old);
	tio_new = tio_old;
	tio_new.c_lflag &= (~ICANON & ~ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &tio_new);

	list_rx = List_create();
	list_tx = List_create();
	sem_init(&list_rx_produce_sem, 0, MAX_ELEMS);
	sem_init(&list_tx_produce_sem, 0, MAX_ELEMS);

	pthread_create(&list_rx_produce_thread, NULL, list_rx_produce, NULL);
	pthread_create(&list_rx_consume_thread, NULL, list_rx_consume, NULL);
	pthread_create(&list_tx_produce_thread, NULL, list_tx_produce, NULL);
	pthread_create(&list_tx_consume_thread, NULL, list_tx_consume, NULL);

	sem_wait(&exit_sem);

	pthread_cancel(list_rx_produce_thread);
	pthread_cancel(list_rx_consume_thread);
	pthread_cancel(list_tx_produce_thread);
	pthread_cancel(list_tx_consume_thread);

	pthread_join(list_rx_produce_thread, NULL);
	pthread_join(list_rx_consume_thread, NULL);
	pthread_join(list_tx_produce_thread, NULL);
	pthread_join(list_tx_consume_thread, NULL);

	free(message_rx);
	free(message_tx);

	close(sfd_local);
	close(sfd_remote);

	// resetting terminal back to default mode
	tcsetattr(STDIN_FILENO, TCSANOW, &tio_old);

	return 0;
}
