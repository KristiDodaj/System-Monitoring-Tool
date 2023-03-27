CC = gcc
CFLAGS = -Wall
OBJ = stats_function.o mySystemStats.o

all: monitor

monitor: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< 

.PHONY: clean
clean:
	rm *.o
