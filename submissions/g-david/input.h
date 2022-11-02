#ifndef _INPUT_H_
#define _INPUT_H_

#include "data.h"
// Gets input from user and adds to send list

void Input_init(struct send_data *data);
void Input_shutdown(void);
void Input_join(void);

#endif