#ifndef USERIO_H_
#define USERIO_H_

#include <pthread.h>
#include <string.h>
#include "list.h"

#define MAXREADSIZE 256 
//Used to limit fgets' read size

//Struct for the arguments needed in keyboardRead and screenWrtie
typedef struct userIO_IN_s userIO_IN;
struct userIO_IN_s{
    List *data;
    pthread_mutex_t *mutex;
    bool *endFlag;
};

void* keyboardRead(void *arg);
//Input is a pointer to a list, allows sharing of list
//Gets the keyboard input when enter is pressed, inserts the input into the provided list

void* screenWrite(void *arg);
//Input is a pointer to a list, allows sharing of list
//From the provided list, gets results of list and puts to the screen

#endif