#ifndef _SEND_UDP_THREAD_H_
#define _SEND_UDP_THREAD_H_

void sendUDPThread_init(void);
void* sendData(void* unused);
void sendUDPThread_shutdown(void);

#endif