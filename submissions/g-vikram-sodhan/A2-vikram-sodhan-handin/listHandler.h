#ifndef _LIST_HANDLER_H_
#define _LIST_HANDLER_H_

#include <stdbool.h>

#include "list.h"

// Initialize global list variables that will be used by threads. return False if error when creating lists
bool initialize_Lists();

// Function for adding to the kbdAndSendList. Will use a mutex to prevent entering critical section
void addToKdbAndSendList(void* itemPtr);

// Function for removing from the kbdAndSendList. Will use same mutex for addToKdbAndSendList to handle critical section
void* removeFromKdbAndSendList();

// Function for adding to the receiveAndPrintList. Will use a mutex to prevent entering critical section
void addToReceiveAndPrintList(void* itemPtr);

// Function for removing from the receiveAndPrintList. Will use same mutex for addToReceiveAndPrintList to handle critical section
void* removeFromReceiveAndPrintListList();

// Function for cleaning up and freeing all data possible
void cleanupLists();

#endif