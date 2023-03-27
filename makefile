CC = gcc
CFLAGS = -Wall
OBJ = stats_functions.o mySystemStats.o stats_functions.h

all: monitor

monitor: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< 

.PHONY: clean
clean:
	rm *.o
