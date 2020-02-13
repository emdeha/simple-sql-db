CC = gcc
CFLAGS = -Wall -Wextra -Werror

.PHONY: all clean

all: main

main: main.c lib/mpc/mpc.c core/parser/parser.c
	$(CC) $(CFLAGS) $^ -g -o main

profile: main.c lib/mpc/mpc.c core/parser/parser.c
	$(CC) $(CFLAGS) $^ -O0 -g -o main

clean:
	rm -f main
