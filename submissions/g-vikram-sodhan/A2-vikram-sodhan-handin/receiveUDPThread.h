#ifndef _RECEIVED_UDP_THREAD_H_
#define _RECEIVED_UDP_THREAD_H_

void receiveUDPThread_init(void);
void* receiveData(void* unused);
void receiveUDPThread_shutdown(void);

#endif