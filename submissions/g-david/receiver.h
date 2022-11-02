#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include "data.h"
// Receives message and adds it to the receive list

void Receiver_init(struct receive_data *data);
void Receiver_shutdown(void);
void Receiver_join(void);

#endif
