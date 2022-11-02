TARGETS = s-talk list userIO socketIO
C = gcc
CFLAGS = -g -std=gnu99 -pthread

all: clean $(TARGETS) link

link:
	$(C) $(CFLAGS) -o s-talk s-talk.o list.o userIO.o socketIO.o

$(TARGETS):
	$(C) $(CFLAGS) -c $@.c

clean:
	rm -rf s-talk *.o