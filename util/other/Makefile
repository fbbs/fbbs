# Generated automatically from Makefile.in by configure.
#  Makefile for local utility tool kit
# 
#  $Id: Makefile 70 2005-09-19 11:01:48Z SpiritRain $
#

CC       = gcc -O2 
#CFLAGS   = -O -Wunused  -I../../include
CFLAGS   = -O2 -export -Wunused -I../include -I../../include -DTHREAD_C
BBSHOME	 = /home/bbs

UTILITY  = check_perm

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c

all: $(UTILITY)

install: $(UTILITY)
	cp $(UTILITY) $(BBSHOME)/bin/@check

clean:
	rm -fr *.o *.BAK *.log $(UTILITY) *core* DEAD* *~

checkspe:
	$(CC) $(CFLAGS) -o checkspe checkspe.c -lcrypt
	
userperm:
	$(CC) $(CFLAGS) -o userperm userperm.c -lcrypt

	
perfcount:
	$(CC) $(CFLAGS) -o perfcount perfcount.c ../../src/record.c -lcrypt -L../../lib -lBBS


