#ifndef _SEND_H_
#define _SEND_H_

//startup send module
void Send_init(struct sockaddr_in* si, int sd);
//shutdown send module
void Send_shutDown();


#endif 