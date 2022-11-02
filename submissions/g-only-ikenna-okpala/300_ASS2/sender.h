#ifndef SENDER_H
#define SENDER_H
void createSendConnection(char * receiverAddr, char * receiverName);
void sendMessage(char * message);
void endSendChatSession();

#endif