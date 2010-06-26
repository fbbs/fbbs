#include <fcntl.h>

#include "fbbs/bbs.h"
#include "fbbs/cache.h"
#include "fbbs/socket.h"
#include "fbbs/user.h"

int load_ucache(ucache_t *ucache, const char *file)
{
	ucache->m.oflag = O_RDWR;
	if (mmap_open(file, &ucache->m) < 0)
		return -1;

	if (hash_create(&ucache->uid_hash, USER_HASH_SLOTS, NULL) < 0) {
		mmap_close(&ucache->m);
		return -1;
	}

	if (hash_create(&ucache->name_hash, USER_HASH_SLOTS, NULL) < 0) {
		mmap_close(&ucache->m);
		hash_destroy(&ucache->uid_hash);
		return -1;
	}

	ucache->begin = ucache->m.ptr;
	ucache->end = ucache->begin + ucache->m.size / sizeof(user_t);
	for (user_t *user = ucache->begin; user != ucache->end; ++user) {
		if (user->uid != 0) {
			hash_set(&ucache->uid_hash, &user->uid, sizeof(user->uid), user);
			hash_set(&ucache->name_hash, user->name, HASH_KEY_STRING, user);
		}
	}

	return 0;
}


