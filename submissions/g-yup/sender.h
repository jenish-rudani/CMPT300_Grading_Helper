#ifndef _SENDER_H_
#define _SENDER_H_

void Sender_init(char* hostName, char* port);
void Sender_signal();
void Sender_cancel();
void Sender_waitForShutdown();

#endif