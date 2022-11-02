#ifndef _RECV_H_
#define _RECV_H_
//startup recv module
void Recv_init(struct sockaddr_in* s, int sd);
//shutdown recv module
void Recv_shutDown();


#endif 