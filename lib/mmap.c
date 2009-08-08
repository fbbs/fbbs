/* Memory mapped I/O */

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/file.h>
#include "libBBS.h"

// Maps whole file 'filename' to memory and locks it.
// Returns the file descriptor on success, -1 on error.
// The size of the file is returned via 'size'.
// 'ptr' is the starting address of mapped region.
// 'flags' should be one of MMAP_RDONLY, MMAP_WRONLY, MMAP_RDWR
// or MMAP_NOLOCK. When MMAP_NOLOCK is specified, fd is closed.
int mmap_open(const char *file, int flags, void **ptr, size_t *size)
{
	int fd, open_flag, mmap_prot, lock_flag;
	struct stat st;
	// Set flags respectively.
	switch (flags) {
		case MMAP_WRONLY:
			open_flag = O_WRONLY;
			mmap_prot = PROT_WRITE;
			lock_flag = LOCK_EX;
			break;
		case MMAP_RDWR:
			open_flag = O_RDWR;
			mmap_prot = PROT_READ | PROT_WRITE;
			lock_flag = LOCK_EX;
			break;
		default: // default: read only
			open_flag = O_RDONLY;
			mmap_prot = PROT_READ;
			lock_flag = LOCK_SH;
			break;
	}	
	// Open and lock the file.
	fd = open(file, open_flag);
	if (fd < 0)
		return -1;
	if (flags != MMAP_NOLOCK)
		flock(fd, lock_flag);
	// Return error if not a regular file or wrong size.
	if ((fstat(fd, &st) < 0) || (!S_ISREG(st.st_mode))
			|| (st.st_size <= 0)) {
		if (flags != MMAP_NOLOCK)
			flock(fd, LOCK_UN);
		close(fd);
		return -1;
	}
	// Map whole file to memory.
	*ptr = mmap(NULL, st.st_size, mmap_prot, MAP_SHARED, fd, 0);
	if (flags == MMAP_NOLOCK)
		close(fd);
	if (*ptr != MAP_FAILED) {
		*size = st.st_size;
		return fd;
	}
	if (flags != MMAP_NOLOCK) {
		flock(fd, LOCK_UN);
		close(fd);
	}
	return -1;
}

// Unmaps memory-mapped region. ('size' bytes starting from 'ptr')
// Unlocks 'fd' if it is a valid file descriptor.
void mmap_close(void *ptr, size_t size, int fd)
{
	munmap(ptr, size);
	if (fd > 0) {
		flock(fd, LOCK_UN);
		close(fd);
	}
}

// Truncate 'fd' to 'newsize'. If file extends, remap whole file.
// On success, return 'fd', otherwise -err.
int mmap_truncate(int fd, size_t newsize, void **ptr, size_t *size)
{
	if (newsize <= 0)
		return -1;
	if (newsize > *size)
		munmap(*ptr, *size);
	if (ftruncate(fd, newsize) < 0)
		return -1;
	if (newsize > *size) {
		*ptr = mmap(NULL, newsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (*ptr == MAP_FAILED)
			return -1;
		*size = newsize;
	}
	return fd;	
}

// Similar to 'mmap_open', but it uses a file descriptor instead.
int safe_mmapfile_handle(int fd, int openflag, int prot, int flag,
		void **ret_ptr, size_t *size)
{
	struct stat st;

	if (fd < 0)
		return 0;
	if ((fstat(fd, &st) < 0)
			|| (!S_ISREG(st.st_mode))
			|| (st.st_size <= 0)) {
		close(fd);
		return 0;
	}
	*ret_ptr = mmap(NULL, st.st_size, prot, flag, fd, 0);
	if (*ret_ptr == NULL)
		return 0;
	*size = st.st_size;
	return 1;
}

