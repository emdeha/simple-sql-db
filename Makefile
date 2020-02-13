CC = gcc
CFLAGS = -Wall -Wextra -Werror
TARGETS = main.c lib/mpc/mpc.c core/**/*.c

.PHONY: all clean

all: main

main: $(TARGETS)
	$(CC) $(CFLAGS) $^ -g -o main

profile: $(TARGETS)
	$(CC) $(CFLAGS) $^ -O0 -g -o main

clean:
	rm -f main
