#Learned and adpated how to make a makefile from https://www.youtube.com/watch?v=_r7i5X0rXJk Username: Paul Programming
output: chat.o threads.o list.o 
	g++ chat.o threads.o list.o  -pthread -o chat

chat.o: chat.c 
	g++ -c chat.c

threads.o: threads.c 
	g++ -c threads.c

list.o: list.c 
	g++ -c list.c

clean:
	rm *.o chat
