CC = gcc
CFLAGS = -Wall -Wextra -Werror

.PHONY: all clean

all: main

main: main.c lib/mpc/mpc.c
	$(CC) $(CFLAGS) $^ -g -o main

clean:
	rm -f main
