CFLAGS = -Wall
OBJFILES = main.o s-talk.o list.o
TARGET = s-talk

all: $(TARGET)

$(TARGET): $(OBJFILES)
	gcc $(CFLAGS) -o $(TARGET) $(OBJFILES)

clean:	
	rm -f $(OBJFILES) $(TARGET) *~ 

