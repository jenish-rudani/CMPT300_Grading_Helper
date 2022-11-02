asn2: main.o list.o
	gcc -o s-talk -pthread main.o list.o

main.o: main.c list.h
	gcc -c -g main.c

list.o: list.c list.h
	gcc -c -g list.c

clean:
	rm -f s-talk *.o
