# Generated automatically from Makefile.in by configure.
#  Makefile for local utility tool kit
# 
#  $Id: Makefile 171 2006-03-07 13:27:59Z root $
#

CC       = gcc
CFLAGS   = -O2 -export -Wunused -I../include -I../../include -DTHREAD_C
BBSHOME	 = /home/bbs

UTILITY  = account averun bbstop bbspop3d birthday horoscope usage_day remake\
	   newacct poststat showuser bbsuptime badbms AnnIndex showbm statBM\
	   countid
	   

.c.o:   ;   $(CC) $(CFLAGS) -c $*.c

all: $(UTILITY)

install: $(UTILITY)
	cp $(UTILITY) $(BBSHOME)/bin/@stat

clean:
	rm -fr *.o *.BAK *.log $(UTILITY) *core* DEAD* *~

bbspop3d:
	$(CC) $(CFLAGS) -o bbspop3d bbspop3d.c -lcrypt

usage: usage.c record.c
	$(CC) $(CFLAGS) -o usage usage.c record.c

usage_day: usage_day.c record.c
	$(CC) $(CFLAGS) -o usage_day usage_day.c record.c
