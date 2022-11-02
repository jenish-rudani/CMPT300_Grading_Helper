#ifndef _PRINTER_H_
#define _PRINTER_H_

#include "data.h"
// Prints from receive list when receiver signals

void Printer_init(struct receive_data *data);
void Printer_shutdown(void);
void Printer_join(void);

#endif