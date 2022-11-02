#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <time.h>


#include "shutdown.h"
#include "keyboard.h"
#include "send.h"
#include "receive.h"
#include "screen.h"
#include "socket.h"


/*
    TODO LIST
    - Test talking to self
    - Cleanup output
    - destroy mutex and condition variable
    - Test process
    [60] Automatic funcitonal testing:
        * [25] Basic: 
            * send data slowly on same computer (10 short lines, ! at end)
            * repeat with both s-talk apps under Valgrind (-10 deduction if leaks memory)
            (-5 deduction if one side fails to exit)
            (may need to test manually if fails on automated input/output redirecting)
        * [5] Network (CSIL)
            * Resolve via IP   - Send data slowly (20 short lines, ! at end)
            * Resolve via name - Send data slowly (20 short lines, ! at end)            
            * Error check: socket in use (-2)
            * Error check: remote computer not found (-2)
        * [15] Fast: send data fast on same computer
            * 90 short lines; ! at end
            * 3 long lines (<= 4000 character); ! at end
            * 20 short lines; ! in middle
        * [15] Bi-directional sending 
            * one PC; bi-directional concurrent bursts of 20 short lines; multiple bursts
            * Repeat with both s-talk apps under Valgrind (-10 deduction if leaks memory)

        About memory leaks:
        - Max -10 for memory leaks between all tests
        - Deduct 5 marks per unique line in code of a leak
        - Ignore any "still reachable" memory reports related to pthread_cancel()

    [40] Look at code (deductions in brackets):
        * Use 4 threads for doing work (may have extra main() thread) (-20 or more)
        * Use List ADT for exchanging data (-10 or more)
        * Reasonable mutex use (-10)
        * Reasonable condition varibale use (-10)
        * Checking return values on getaddrinfo(), socket(), etc. (-10)
        * Closing socket, freeing mutex & condition variables (-10)

*/
static void sleep_ms(int duration_ms);

int main(int argc, char** args) 
{
    #if 1
    if (argc != 4) {
        char* exeName = args[0];
        printf("Usage:\n");
        printf("  %s <local port> <remote host> <remote port>\n", exeName);
        printf("Examples:\n");
        printf("  %s 3000 192.168.0.513 3001\n", exeName);
        printf("  %s 3000 some-computer-name 3001\n", exeName);
        exit(EXIT_SUCCESS);
    }

    char* localPortStr = args[1];
    char* remoteHostStr = args[2];
    char* remotePortStr = args[3];
    #else

        printf("\n\nWARNING: HARD CODING ARGUMENTS FOR TESTING\n\n\n");
        char* localPortStr = "3000";
        char* remoteHostStr = "localhost";
        char* remotePortStr = "3001";
    #endif

    // Initialize
    Shutdown_init();
    Socket_init(localPortStr, remoteHostStr, remotePortStr);
    Send_init();
    Screen_init();
    Receive_init();
    Keyboard_init();

    // Wait
    printf("Welcome to S-Talk! Please type your messages now.\n");
    Shutdown_waitForShutdown();

    // Cleanup
    sleep_ms(500);
    Keyboard_shutdown();
    Receive_shutdown();
    Send_shutdown();
    Screen_shutdown();
    Socket_shutdown();
}

/***********************************************************
 * Utility Functions
 ***********************************************************/
static void sleep_ms(int duration_ms)
{
	struct timespec req;
	req.tv_nsec = (duration_ms % 1000) * 1000 * 1000;
	req.tv_sec = (duration_ms / 1000);
	nanosleep(&req, NULL);
}
