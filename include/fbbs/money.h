#ifndef FB_MONEY_H
#define FB_MONEY_H

#include <inttypes.h>
#include <stdint.h>

#define PRIdMONEY  PRId64
#define DBIdMONEY  "l"
typedef int64_t  money_t;

#ifdef ENABLE_BANK
# define TO_CENTS(y)  (y * 100)
# define TO_YUAN(c)  (c / 100.0L)
# define TO_YUAN_INT(c)  ((int)(c / 100.0L))
# define PERCENT_RANK(r)  (((int)(r * 1000)) / 10.0)
#endif // ENABLE_BANK

#endif // FB_MONEY_H
