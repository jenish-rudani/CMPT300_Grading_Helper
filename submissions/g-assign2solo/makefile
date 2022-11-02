CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build:
	gcc $(CFLAGS) main.c receiver.c screen.c keyboard.c sender.c list.c general.c -lpthread -o s-talk

run: build
	./s-talk 22110 localhost 22111

valgrind: build
	valgrind --leak-check=full ./s-talk 22110 localhost 22111

clean:
	rm -f s-talk