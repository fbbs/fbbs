#ifndef FB_PASS_H
#define FB_PASS_H

#include <stdbool.h>

extern const char *generate_passwd(const char *pw);
extern bool check_passwd(const char *pw_crypted, const char *pw_try);

#endif // FB_PASS_H
