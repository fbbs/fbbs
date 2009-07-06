CC       = gcc
CFLAGS   = -O2 -Wall -I../../include
BBSHOME	 = /home/bbs
LIBS     = -lBBS -L../../lib

UTILITY  = averun bbstop bbspop3d birthday horoscope usage_day remake\
	   newacct poststat showuser bbsuptime badbms AnnIndex showbm statBM\
	   countid

all: $(UTILITY) charts

install: $(UTILITY) charts
	cp $(UTILITY) $(BBSHOME)/bin/@stat

charts: chart.o account.o
	$(CC) $(CFLAGS) account.o chart.o -o account $(LIBS)

clean:
	rm -fr *.o *.BAK *.log $(UTILITY) *core* DEAD* *~
	rm -rf account

bbspop3d:
	$(CC) $(CFLAGS) -o bbspop3d bbspop3d.c -lcrypt

usage: usage.c record.c
	$(CC) $(CFLAGS) -o usage usage.c record.c

usage_day: usage_day.c record.c
	$(CC) $(CFLAGS) -o usage_day usage_day.c record.c

%.o: %.c
	$(CC) $(CFLAGS) $< -c

% :: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
