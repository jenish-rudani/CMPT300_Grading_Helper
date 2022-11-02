#ifndef _SHUTDOWNMANAGER_H_
#define _SHUTDOWNMANAGER_H_

#include "input.h"
#include "sender.h"
#include "receiver.h"
#include "printer.h"
// For threads to shutdown

void ShutdownManagerForSender_triggerShutdown();
void ShutdownManagerForReceiver_triggerShutdown();

#endif