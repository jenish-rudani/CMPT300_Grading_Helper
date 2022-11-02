#include <stdio.h>
#include <stdlib.h>
#include "includes/sockets.h"
#include "includes/list.h"

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fputs("Missing Inputs:\n Enter as: './s-talk [local port] [remote server] [remote port]'\n", stdout);
        exit(1);
    }
    else
    {
        fputs("Starting s-talk\n", stdout);
    }

    // startup my modules
    Sockets_init(argv[1], argv[2], argv[3]);

    printf("'!' to exit\n");

    // shutdown my modules
    Sockets_shutdown();
    printf("done\n");

    return 0;
}
