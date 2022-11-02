s-talk: s-talk.o sockets.o receiver.o sender.o
	gcc -g -o s-talk -pthread s-talk.o list.o sockets.o receiver.o sender.o -lm

s-talk.o: s-talk.c
	gcc -g -c s-talk.c

sockets.o: sockets.c
	gcc -g -c sockets.c 

receiver.o: receiver.c
	gcc -g -c receiver.c

sender.o: sender.c
	gcc -g -c sender.c

run:
	make
	./main

valgrind:
	valgrind --leak-check=full ./main

clean:
	rm s-talk.o receiver.o sender.o sockets.o s-talk

