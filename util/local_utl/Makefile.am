AM_CFLAGS = -O2 -pipe -std=c99 -Wall @ENABLE_DEBUG@
AM_CPPFLAGS = -D_XOPEN_SOURCE=600 -I../../include
LDADD = ../../lib/libBBS.a
bindir = ${prefix}/tools

bin_PROGRAMS = bbstop birthday toppost bbsuptime badbms AnnIndex showbm \
               statBM averun account newacct horoscope zodiac

averun_SOURCES = averun.c chart.c
account_SOURCES = account.c chart.c
newacct_SOURCES = newacct.c chart.c
horoscope_SOURCES = horoscope.c chart.c
zodiac_SOURCES = zodiac.c chart.c
