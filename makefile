CC = gcc
CFLAGS = -Wall
OBJ = stats_function.c mySystemStats.C

all: monitor

monitor: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< 

.PHONY: clean
clean:
	rm *.o
