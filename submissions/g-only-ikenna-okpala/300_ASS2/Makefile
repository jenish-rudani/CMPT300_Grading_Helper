s-talk: s-talk.o list.o sender.o receiver.o interpreter.o threads.o
	gcc -o s-talk s-talk.o list.o interpreter.o sender.o receiver.o threads.o -pthread

s-talk.o: s-talk.c
	gcc -c s-talk.c

list.o: list.c
	gcc -c list.c

interpreter.o: interpreter.c
	gcc -c interpreter.c

sender.o: sender.c
	gcc -c sender.c

receiver.o: receiver.c
	gcc -c receiver.c

threads.o: threads.c
	gcc -c threads.c

clean:
	rm *.o s-talk