#include "shutdownManager.h"

// Shutdown all threads except sender because sender will exit on its own
void ShutdownManagerForSender_triggerShutdown()
{
    Input_shutdown();
    Receiver_shutdown();
    Printer_shutdown();
}

// Shutdown all threads except sender because receiver will exit on its own
void ShutdownManagerForReceiver_triggerShutdown()
{
    Input_shutdown();
    Sender_shutdown();
    Printer_shutdown();
}
