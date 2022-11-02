CFLAGS = -IC -g -std=c99 -Wall -Werror
CLIBS = -lpthread

all: s-talk

s-talk:
	gcc s-talk.c modules/input.c modules/output.c modules/receiver.c modules/sender.c modules/socket.c manager/shutdownManager.c list/list.o $(CFLAGS) $(CLIBS) -o $@

clean:
	rm -rf s-talk