CC = gcc
CFLAGS = -Wall -Wextra -g

wish: wish.c
	$(CC) $(CFLAGS) -o wish wish.c

clean:
	rm -f wish