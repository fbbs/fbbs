#ifndef FB_UINFO_H
#define FB_UINFO_H

#include "bbs.h"

enum {
	UINFO_ENICK = -1,
	UINFO_EGENDER = -2,
	UINFO_EBIRTH = -3,
};

extern char *cexpstr(int exp);
#ifdef ALLOWGAME
extern char *cnummedals(int num);
extern char *cmoney(int num);
#endif
extern char *cperf(int perf);
extern int countexp(const struct userec *udata);
extern int countperf(const struct userec *udata);
extern int julian_day(int year, int month, int day);
extern int days_elapsed(int year, int month, int day, time_t now);
extern const char *horoscope(char month, char day);
extern int compute_user_value(const struct userec *urec);
extern void show_position(const struct userec *user, char *buf, size_t size);
extern int check_user_profile(const struct userec *u);

#endif // FB_UINFO_H
