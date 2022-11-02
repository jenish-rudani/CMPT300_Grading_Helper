#ifndef RECEIVER_H
#define RECEIVER_H
void createReceiveConnection(char * receiverPort);
char * receiveMessage();
void endRecChatSession();
#endif