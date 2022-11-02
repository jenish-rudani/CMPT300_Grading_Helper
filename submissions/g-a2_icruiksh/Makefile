
FILE_LIST = main.c s_talk.c list.c
CFLAGS = -g -pthread
TARGET = s-talk

# Default: builds the project
all: build

# Runs the main function
run: build
	./$(TARGET)

# Link main.o and list.o to executable main
build: clean
	gcc $(CFLAGS) $(FILE_LIST) -o $(TARGET)

# Remove built files
clean:
	rm -rf $(TARGET)