CC = gcc
CFLAGS = -Wall -g
SOURCES = main.c file_manager.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = file_manager

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install:
	cp $(EXECUTABLE) /usr/local/bin/
