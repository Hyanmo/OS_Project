CC = gcc
CFLAGS = -Wall -g
TARGET = file_manager
OBJ = file_manager.o main.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

file_manager.o: file_manager.c file_manager.h
	$(CC) $(CFLAGS) -c file_manager.c

main.o: main.c file_manager.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f $(OBJ) $(TARGET)
