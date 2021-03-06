CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Werror
OPTFLAG = -O2
DEBUGFLAG = -g

PROGRAM = lc3sim

all : build

build : CFLAGS += $(OPTFLAG)
build : $(PROGRAM)

$(PROGRAM) : lc3sim.o lc3.o
	gcc lc3sim.o lc3.o -o $(PROGRAM)
	
lc3sim.o : lc3sim.c lc3.h
	gcc $(CFLAGS) -c lc3sim.c
	
lc3.o : lc3.h lc3.c
	gcc $(CFLAGS) -c lc3.c

clean:
	rm -f *.o $(PROGRAM)

# Simple debug target
debug : CFLAGS += $(DEBUGFLAG) 
debug : clean $(PROGRAM)

.PHONY : all build clean
