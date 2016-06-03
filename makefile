# the compiler to use.
CC=gcc
# options 
CFLAGS=-c -Wall -std=c99 -D_GNU_SOURCE

all: shellp

shellp: shellp.o
	$(CC) shellp.o -o shellp

shellp.o: shellp.c
	$(CC) $(CFLAGS) shellp.c

clean:
	rm *o shellp
