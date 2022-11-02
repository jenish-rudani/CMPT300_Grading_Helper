CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build:
	gcc $(CFLAGS) main.c keyboard.c printer.c receiver.c sender.c list.o -lpthread -o s-talk

valgrind: build
	valgrind --leak-check=full ./s-talk 22110 22110 22110

clean:
	rm -f s-talk