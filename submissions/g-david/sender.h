#ifndef _SENDER_H_
#define _SENDER_H_

#include "data.h"
// Gets message from send list and sends it

void Sender_init(struct send_data *data);
void Sender_shutdown(void);
void Sender_join(void);

#endif