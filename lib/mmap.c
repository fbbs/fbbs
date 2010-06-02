/** @file
 * Memory mapped I/O.
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/file.h>
#include "libBBS.h"
#include "mmap.h"

enum {
	MMAP_MINSIZE = 4096,  ///< Minimum memory mapped region size.
};

/**
 * Map file associated with file descriptor to memory.
 * @param[in,out] m pointer to an ::mmap_t struct (should be set properly).
 * @return 0 on success, -1 on error.
 * @attention This function is exported only for compatability. It will be
 *            made private sooner or later.
 */
int mmap_open_fd(mmap_t *m)
{
	struct stat st;

	if (m->fd < 0)
		return -1;
	if (fb_flock(m->fd, m->lock) < 0) {
		close(m->fd);
		return -1;
	}

	// Return error if 'file' is not a regular file or has a wrong size.
	if (fstat(m->fd, &st) < 0 || !S_ISREG(st.st_mode)
			|| st.st_size < 0) {
		fb_flock(m->fd, LOCK_UN);
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
	fb_flock(m->fd, LOCK_UN);
	close(m->fd);
	return -1;
}

/**
 * Map file to memory and set appropriate lock on it.
 * @param[in] file file to be mmap'ed.
 * @param[in,out] m mmap struct pointer.
 * @return 0 on success, -1 on error.
 * @note m->oflag should be properly set in advance.
 */
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

/**
 * Unmap memory-mapped file.
 * Related lock is released and file descriptor is closed.
 * @param[in] m pointer to an ::mmap_t struct.
 * @return 0 on success, -1 on error.
 */
int mmap_close(mmap_t *m)
{
	if (m == NULL)
		return 0;
	munmap(m->ptr, m->msize);
	if (m->lock != LOCK_UN)
		fb_flock(m->fd, LOCK_UN);
	return restart_close(m->fd);
}

/**
 * Truncate mmap'ed file to new size.
 * If file extends, remap whole file.
 * @param[in,out] m pointer to an ::mmap_t struct.
 * @param[in] size new file size.
 * @return 0 on success, -1 on error.
 */
int mmap_truncate(mmap_t *m, size_t size)
{
	if (size < 0) {
		mmap_close(m);
		return -1;
	}
	if (size > m->msize)
		munmap(m->ptr, m->size);
	if (restart_ftruncate(m->fd, size) < 0) {
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

/**
 * Shrink memory mapped file to new size.
 * @param[in] m pointer to an ::mmap_t struct.
 * @param[in] size new file size, must be smaller than current size.
 * @return 0 on success, -1 on error.
 */
int mmap_shrink(mmap_t *m, size_t size)
{
	if (size >= m->msize)
		return -1;
	if (restart_ftruncate(m->fd, size) < 0)
		return -1;
	m->size = size;
	return 0;
}

/**
 * Change lock on a memory mapped file.
 * @param[in,out] m pointer to an ::mmap_t struct.
 * @param[in] lock new lock state. (LOCK_EX LOCK_SH LOCK_UN)
 */
int mmap_lock(mmap_t *m, int lock)
{
	if (lock == m->lock)
		return 0;
	m->lock = lock;
	return fb_flock(m->fd, lock);
}
