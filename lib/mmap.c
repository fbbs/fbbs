/* Memory mapped I/O */

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/file.h>

// Maps whole file 'filename' to memory and locks it.
// File descriptor of the file is returned via 'ret_fd'.
// The size of the file is returned via 'size'.
// 'openflag' is passed to open().
// 'prot', 'flag' are passed to mmap().
// 'ret_ptr' is the starting address of mapped region.
// Returns 1 if OK, 0 on error.
int safe_mmapfile(const char *filename, int openflag, int prot, int flag,
		void **ret_ptr, size_t *size, int *ret_fd)
{
	int fd;
	struct stat st;

	fd = open(filename, openflag, 0600);
	if (fd < 0)
		return 0;
	// Return error if not a regular file or wrong size.
	if ((fstat(fd, &st) < 0)
			|| (!S_ISREG(st.st_mode))
			|| (st.st_size <= 0)) {
		close(fd);
		return 0;
	}

	// Map whole file to memory.
	*ret_ptr = mmap(NULL, st.st_size, prot, flag, fd, 0);
	if (!ret_fd) {
		close(fd);
	} else {
		*ret_fd = fd;
		flock(fd, LOCK_EX);
	}
	if (*ret_ptr == NULL)
		return 0;
	*size = st.st_size;
	return 1;
}

// Similar to 'safe_mmapfile', but it uses a file descriptor instead.
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

// Unmaps memory-mapped region. ('size' bytes starting from 'ptr')
// Unlocks 'fd' if it is a valid file descriptor.
void end_mmapfile(void *ptr, size_t size, int fd)
{
	munmap(ptr, size);
	if (fd != -1) {
		flock(fd, LOCK_UN);
		close(fd);
	}
}

