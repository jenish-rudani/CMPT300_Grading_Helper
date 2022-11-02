#ifndef _PRINT_SCREEN_THREAD_H_
#define _PRINT_SCREEN_THREAD_H_

void printScreenThread_init(void);
void* printToScreen(void* unused);
void printScreenThread_shutdown(void);

#endif