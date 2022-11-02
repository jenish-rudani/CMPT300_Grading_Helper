all:
	gcc -c -Wall -Werror stalk.c list.c
	gcc -g -Wall -Werror -o stalk -pthread stalk.o list.o
clean:
	-rm -f *.o stalk
