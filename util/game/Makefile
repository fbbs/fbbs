# Generated automatically from Makefile.in by configure.
BBSHOME=/home/bbs

CC      = gcc
CFLAGS  = -O
DEFINES = -I../../include/

CGAMES 	 = chicken.c game.c money.c
OBJGAMES = chicken.o game.o money.o

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) -c $*.c

all: winmine game.so

winmine: winmine.c
	$(CC) -o winmine winmine.c

game.so: $(OBJGAMES)
	ld -s -L../../lib -lBBS -G $(OBJGAMES) -o game.so

install: game.so winmine
	cp game.so $(BBSHOME)/so
	cp winmine $(BBSHOME)/so/winmine.new
	-mv $(BBSHOME)/so/winmine $(BBSHOME)/so/winmine.old
	mv $(BBSHOME)/so/winmine.new $(BBSHOME)/so/winmine

clean:
	-rm -f *~ *.o DEADJOE *.so winmine

