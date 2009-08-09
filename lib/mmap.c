/* Memory mapped I/O */

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/file.h>
#include "libBBS.h"

enum {
	MMAP_MINSIZE = 4096,
};

int mmap_open_fd(mmap_t *m)
{
	struct stat st;

	if (m->fd < 0)
		return -1;
	if (flock(m->fd, m->lock) < 0) {
		close(m->fd);
		return -1;
	}

	// Return error if 'file' is not a regular file or has a wrong size.
	if (fstat(m->fd, &st) < 0 || !S_ISREG(st.st_mode)
			|| st.st_size < 0) {
		flock(m->fd, LOCK_UN);
		close(m->fd);
		return -1;
	}

	m->size = st.st_size;
	m->mflag = MAP_SHARED;
	// Prevent 0-length mapping error.
	m->msize = st.st_size > MMAP_MINSIZE ? st.st_size : MMAP_MINSIZE;

	// Map whole file to memory.
	m->ptr = mmap(NULL, m->msize, m->prot, m->mflag, m->fd, 0);
	if (m->ptr != MAP_FAILED)
		return 0;
	flock(m->fd, LOCK_UN);
	close(m->fd);
	return -1;
}

// Map whole 'file' to memory and locks it.
// Return 0 and set 'm' on success, -1 on error.
// m->oflag should be properly set before calling the function.
int mmap_open(const char *file, mmap_t *m)
{
	// Set mmap and lock flags.
	m->lock = LOCK_EX;
	if (m->oflag & O_RDWR) {
		m->prot = PROT_READ | PROT_WRITE;
	} else if (m->oflag & O_WRONLY) {
		m->prot = PROT_WRITE;
	} else {
		m->prot = PROT_READ;
		m->lock = LOCK_SH;
	}

	m->fd = open(file, m->oflag);
	return mmap_open_fd(m);
}

// Unmap memory-mapped file, unlock and close fd.
int mmap_close(mmap_t *m)
{
	munmap(m->ptr, m->size);
	if (m->lock != LOCK_UN)
		flock(m->fd, LOCK_UN);
	return close(m->fd);
}

// Truncate mmaped file to 'size'. If file extends, remap whole file.
// Return 0 on success, -1 on error.
int mmap_truncate(mmap_t *m, size_t size)
{
	if (size < 0) {
		mmap_close(m);
		return -1;
	}
	if (size > m->msize)
		munmap(m->ptr, m->size);
	if (ftruncate(m->fd, size) < 0) {
		mmap_close(m);
		return -1;
	}
	m->size = size;
	if (size > m->msize) {
		m->ptr = mmap(NULL, size, m->prot, m->mflag, m->fd, 0);
		if (m->ptr == MAP_FAILED) {
			mmap_close(m);
			return -1;
		}
		m->msize = size;
	}
	return 0;
}

// Change lock on underlying file handle.
int mmap_lock(mmap_t *m, int lock)
{
	if (lock == m->lock)
		return 0;
	m->lock = lock;
	return flock(m->fd, lock);
}
