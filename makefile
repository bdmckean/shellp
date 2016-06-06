# the compiler to use.
CC=gcc
# options 
CFLAGS=-c -Wall -std=c99 -D_GNU_SOURCE

all: shellp 

shellp: shellp.o commands.o parseline.o
	$(CC) shellp.o commands.o parseline.o -o shellp

shellp.o: shellp.c shellp.h commands.h
	$(CC) $(CFLAGS) shellp.c

commands.o: commands.c commands.h
	$(CC) $(CFLAGS) commands.c

parseline.o: parseline.c 
	$(CC) $(CFLAGS) parseline.c

clean:
	rm *o shellp
