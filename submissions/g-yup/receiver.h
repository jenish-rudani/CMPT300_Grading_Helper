#ifndef _RECEIVER_H_
#define _RECEIVER_H_

void Receiver_init();
void Receiver_addMessageInList(char* message);
char* Receiver_pickMessageInList();
void Receiver_cancel();
void Receiver_waitForShutdown();

#endif