// Handle user cache.

#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>

struct UTMPFILE *utmpshm = NULL;
struct UCACHE *uidshm = NULL;
struct userec lookupuser;
