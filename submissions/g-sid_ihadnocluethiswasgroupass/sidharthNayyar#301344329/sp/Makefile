#make file
 
CC=gcc  #compiler
CFLAGS  = -g -Wall
LDFLAGS=-lpthread -lcurses
TARGET=s-talk #target file name
 
all:
	$(CC) $(CFLAGS) main.c utils.c list.c -o $(TARGET) $(LDFLAGS)
 
clean:
	rm $(TARGET)
