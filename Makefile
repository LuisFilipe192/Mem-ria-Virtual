CC = gcc

CFLAGS = -Wall -Wextra -std=c99 -O2

all: vm

vm: vm.c
	$(CC) $(CFLAGS) -o vm vm.c -pthread

clean:
	rm -f vm 

.PHONY: all clean