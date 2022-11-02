# S-Talk makefile
#
# make listLibraryBuild
#	Creates the content to distribute to students in the $(DISTRIBUTE_DIR) folder

DISTRIBUTE_DIR=./distribute
SOURCES=main.c receive.c keyboard.c screen.c shutdown.c send.c list.c socket.c
OPTIONS=-g -std=c11 -Wall -Werror -D _POSIX_C_SOURCE=200809L

all: buildSol 

buildSol:
	gcc $(OPTIONS) -o s-talk $(SOURCES) -lpthread

run:
	./s-talk 3000 localhost 3001


valgrind:
	# valgrind --show-leak-kinds=all --leak-check=full ./s-talk
	valgrind --leak-check=full ./s-talk

test_netcat_listen:
	netcat -l -u 3001
test_netcat_send:
	netcat -u localhost 3000

# listLibraryBuild:
#	REMOVED BECAUSE MY LIST IS NOW THREADSAFE
# 	mkdir -p $(DISTRIBUTE_DIR)
# 	gcc -c -std=c11 -O3 -fvisibility=hidden list.c  -Wall -Werror -o $(DISTRIBUTE_DIR)/instructorList.o
# 	gcc --version > $(DISTRIBUTE_DIR)/gcc_version.txt
# 	cp list.h sampleTest.c $(DISTRIBUTE_DIR)/
# 	echo 'buildTest:\n	gcc -std=c11 -o sampleTest instructorList.o sampleTest.c -Wall -Werror' > $(DISTRIBUTE_DIR)/makefile

listLibraryTest:
	gcc -std=c11 -o sampleTest $(DISTRIBUTE_DIR)/instructorList.o sampleTest.c -Wall -Werror

clean:
	rm -f sampleTest s-talk
