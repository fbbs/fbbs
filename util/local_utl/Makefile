CC		= gcc
CFLAGS	= -O2 -Wall -I../../include
BBSHOME	= /home/bbs
LIBS	= -lBBS -L../../lib

UTILITY	= bbstop bbspop3d birthday horoscope usage_day remake\
		  newacct poststat showuser bbsuptime badbms AnnIndex showbm statBM\
		  countid
UTIL2	= averun account newacct horoscope zodiac

all: $(UTILITY) $(UTIL2)

install: $(UTILITY) $(UTIL2)
	cp $(UTILITY) $(UTIL2) $(BBSHOME)/bin/@stat

charts: $(UTIL2)

$(UTIL2): %: %.o chart.o
	$(CC) $(CFLAGS) $< chart.o -o $@ $(LIBS)

clean:
	rm -fr *.o $(UTILITY) $(UTIL2)

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
