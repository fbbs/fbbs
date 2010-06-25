#include "fbbs/bbs.h"
#include "fbbs/mmap.h"
#include "fbbs/socket.h"
#include "fbbs/user.h"

int load_ucache(mmap_t *m, const char *file)
{
	return mmap_open(file, m);
}


