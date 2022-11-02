#include "list.h"
#include "keyboard.h"
#include "sender.h"
#include "receiver.h"
#include "printer.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if(argc != 4) {
        printf("The number of Argument should be 3.\n");
        printf("First : my port number\n");
        printf("Second : remote machine name\n");
        printf("Third : remote port number\n");
        return -1;
    }
    printf("Simple-Talk has started -> Maximum Character : 100\n");

    // Start threads
    Keyboard_init();
    Sender_init(argv[2], argv[3]);
    Receiver_init(argv[1]);
    Printer_init();

    // Clean up threads
    Keyboard_waitForShutdown();
    Sender_waitForShutdown();
    Receiver_waitForShutdown();
    Printer_waitForShutdown();

    printf("Simple-Talk has ended. Thank you!\n");
    return 0;
}