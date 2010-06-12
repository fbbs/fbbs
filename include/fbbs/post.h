#ifndef FB_POST_H
#define FB_POST_H

#include "fbbs/util.h"

typedef struct post_t {
	seq_t id;
	seq_t reid;
	seq_t gid;
	seq_t owner;
	seq_t eraser;
	uint_t prop;
	uint_t size;
	varchar_t title;
	fb_time_t date;
	fb_time_t deldate;
} post_t;

#endif // FB_POST_H
