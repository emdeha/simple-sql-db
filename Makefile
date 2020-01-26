CC = gcc
CFLAGS = -Wall -Wextra -Werror

.PHONY: all clean

all: main

main: main.c lib/mpc/mpc.c
	$(CC) $(CFLAGS) $^ -o main

clean:
	rm -f main
