CC = gcc
CFLAGS = -Wall
OBJ = stats_functions.o main.o stats_functions.h

all: monitor

monitor: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< 

.PHONY: clean
clean:
	rm *.o

