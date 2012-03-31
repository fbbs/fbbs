#ifndef FB_UINFO_H
#define FB_UINFO_H

#include "bbs.h"
#include "fbbs/dbi.h"

enum {
	UINFO_ENICK = -1,
	UINFO_EGENDER = -2,
	UINFO_EBIRTH = -3,
};

typedef struct uinfo_t {
	db_res_t *res;
	const char *title;
#ifdef ENABLE_BANK
	int64_t contrib;
	int64_t money;
	float rank;
#endif
} uinfo_t;

extern int uinfo_load(const char *name, uinfo_t *u);
extern void uinfo_free(uinfo_t *u);

extern const char *cexpstr(int exp);
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
extern void show_position(const struct userec *user, char *buf, size_t size, const char *title);
extern int check_user_profile(const struct userec *u);
extern int update_user_stay(struct userec *u, bool is_login, bool is_dup);
extern void tui_check_uinfo(struct userec *u);
#endif // FB_UINFO_H
