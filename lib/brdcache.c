#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>

struct BCACHE *brdshm = NULL;
struct boardheader *bcache = NULL;
int numboards = -1;
