/** @file
 * Memory mapped I/O.
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/file.h>
#include "mmap.h"
#include "fbbs/fileio.h"

enum {
	MMAP_MINSIZE = 4096,  ///< Minimum memory mapped region size.
};

/**
 * Map file associated with file descriptor to memory.
 * @param[in,out] m pointer to an ::mmap_t struct (should be set properly).
 * @return 0 on success, -1 on error.
 */
int mmap_open_fd(mmap_t *m)
{
	m->lock = FILE_WRLCK;
	if (m->oflag & O_RDWR) {
		m->prot = PROT_READ | PROT_WRITE;
	} else if (m->oflag & O_WRONLY) {
		m->prot = PROT_WRITE;
	} else {
		m->prot = PROT_READ;
		m->lock = FILE_RDLCK;
	}

	if (m->fd < 0)
		return -1;
	if (file_lock_all(m->fd, m->lock) < 0) {
		close(m->fd);
		return -1;
	}

	struct stat st;
	// Return error if 'file' is not a regular file or has a wrong size.
	if (fstat(m->fd, &st) < 0 || !S_ISREG(st.st_mode)
			|| st.st_size < 0) {
		file_lock_all(m->fd, FILE_UNLCK);
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
	file_lock_all(m->fd, FILE_UNLCK);
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
	m->fd = open(file, m->oflag, 0640);
	return mmap_open_fd(m);
}

void mmap_unmap(mmap_t *m)
{
	if (m) {
		munmap(m->ptr, m->msize);
		if (m->lock != FILE_UNLCK)
			file_lock_all(m->fd, FILE_UNLCK);
	}
}

/**
 * Unmap and close memory-mapped file.
 * Related lock is released and file descriptor is closed.
 * @param[in] m pointer to an ::mmap_t struct.
 * @return 0 on success, -1 on error.
 */
int mmap_close(mmap_t *m)
{
	mmap_unmap(m);
	return file_close(m->fd);
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
	bool remap = size > m->msize;
	if (remap)
		munmap(m->ptr, m->size);
	if (file_truncate(m->fd, size) < 0) {
		if (remap) {
			if (m->lock != FILE_UNLCK)
				file_lock_all(m->fd, FILE_UNLCK);
			file_close(m->fd);
		} else {
			mmap_close(m);
		}
		return -1;
	}
	m->size = size;
	if (remap) {
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
	if (file_truncate(m->fd, size) < 0)
		return -1;
	m->size = size;
	return 0;
}

/**
 * Change lock on a memory mapped file.
 * @param[in,out] m pointer to an ::mmap_t struct.
 * @param[in] lock new lock state.
 */
int mmap_lock(mmap_t *m, file_lock_e lock)
{
	if (lock == m->lock)
		return 0;
	m->lock = lock;
	return file_lock_all(m->fd, lock);
}
