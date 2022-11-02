
client: listSock.o list.o sockeclientt.o 
	gcc list.o listSock.o sockeclientt.o -g -pthread -o client
	-rm sockeclientt.o 
	-rm listSock.o 

server: listSock.o list.o socketserver.o 
	gcc list.o listSock.o socketserver.o -g -pthread -o socketserver -o server
	-rm socketserver.o 
	-rm listSock.o 

sockeclientt.o: sockeclientt.c
	gcc -c sockeclientt.c -g -pthread

socketserver.o: socketserver.c
	gcc -c socketserver.c -g -pthread

listSock.o: listSock.c listSock.h
	gcc -c listSock.c -g

list.o:  list.h

clean:
	-rm a.out
