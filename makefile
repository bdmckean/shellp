# the compiler to use.
CC=gcc
# options 
CFLAGS=-c -Wall -std=c99 -D_GNU_SOURCE

all: shellp 

shellp: shellp.o commands.o parseline.o exec_cmd.o fc.o
	$(CC) shellp.o commands.o parseline.o exec_cmd.o fc.o -o shellp

shellp.o: shellp.c shellp.h commands.h
	$(CC) $(CFLAGS) shellp.c

commands.o: commands.c commands.h
	$(CC) $(CFLAGS) commands.c

parseline.o: parseline.c shellp.h 
	$(CC) $(CFLAGS) parseline.c

exec_cmd.o: exec_cmd.c shellp.h 
	$(CC) $(CFLAGS) exec_cmd.c

fc.o: fc.c shellp.h 
	$(CC) $(CFLAGS) fc.c
clean:
	rm *o shellp
