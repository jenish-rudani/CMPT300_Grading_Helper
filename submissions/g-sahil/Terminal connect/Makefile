FLAGS = -Wall -g -fsanitize=address

all: list s-talk.c
	gcc -pthread -o s-talk s-talk.c list.o $(FLAGS)

list: list.c list.h
	gcc -c list.c $(FLAGS)

clean:
	rm s-talk list.o