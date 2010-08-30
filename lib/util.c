#include <fcntl.h>
#include "fbbs/util.h"

/**
 * Read from /dev/urandom.
 * @param buf The buffer.
 * @param size Bytes to read.
 * @return 0 if OK, -1 on error.
 */
int read_urandom(void *buf, size_t size)
{
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return -1;
	if (read(fd, buf, size) < size)
		return -1;
	close(fd);
	return 0;
}

/**
 * Get an positive int from /dev/urandom.
 * @return A random integer.
 */
int urandom_pos_int(void)
{
	int i;
	read_urandom(&i, sizeof(i));
	if (i < 0)
		return -i;
	return i;
}
