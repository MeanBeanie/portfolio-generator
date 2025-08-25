CFLAGS = -Wall -Wextra -std=c11 -ggdb

all:
	gcc *.c -o portgen $(CFLAGS)
