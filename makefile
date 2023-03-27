CC = gcc
CFLAGS = -Wall
OBJ = stats_function.o mySystemStats.o stats_function.h

all: monitor

monitor: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< 

.PHONY: clean
clean:
	rm *.o
