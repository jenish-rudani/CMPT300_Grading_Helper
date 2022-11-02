#ifndef _SHUTDOWN_H_
#define _SHUTDOWN_H_
#include <stdbool.h>

extern const char* SHUTDOWN_MESSAGE;

void Shutdown_init();

void Shutdown_waitForShutdown();
void Shutdown_triggerShutdown();
bool Shutdown_inShuttingDwon();

void exitOnFail(bool isSuccessful, char* msg);

#endif