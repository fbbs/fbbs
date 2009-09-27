#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

static int rm_dir();

/**
 * Append to file.
 * @param file file name.
 * @param msg a NUL terminated string.
 * @return 0 on success, -1 on error.
 */
int file_append(const char *file, const char *msg)
{
	if (file == NULL || msg == NULL)
		return -1;
	int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd != -1) {
		if (flock(fd, LOCK_EX) == -1) {
			close(fd);
			return -1;
		}
		write(fd, msg, strlen(msg));
		flock(fd, LOCK_UN);
		close(fd);
	}
	return 0;
}

/**
 * Write given bytes to file.
 * @param fd file handle.
 * @param buf starting address of the buffer.
 * @param size bytes to write.
 * @return 0 on success, -1 on error.
 */
int safer_write(int fd, const void *buf, int size)
{
	int cc, sz = size;
	const char *bp = buf;

	do {
		cc = write(fd, bp, sz);
		if ((cc < 0) && (errno != EINTR)) {
			report("safer_write() err!", "");
			return -1;
		}
		if (cc > 0) {
			bp += cc;
			sz -= cc;
		}
	} while (sz > 0);
	return 0;
}

/**
 * Check whether given file is a regular file.
 * @param file file name.
 * @return 1 if file exists and is a regular file, 0 otherwise.
 */
int dashf(const char *file)
{
	if (file == NULL)
		return 0;
	struct stat st;
	return (stat(file, &st) == 0 && S_ISREG(st.st_mode));
}

/**
 * Check whether given file is a directory.
 * @param file file name (directory path).
 * @return 1 if file exists and is a directory, 0 otherwise.
 */
int dashd(const char *file)
{
	if (file == NULL)
		return 0;
	struct stat st;
	return (stat(file, &st) == 0 && S_ISDIR(st.st_mode));
}

/* mode == O_EXCL / O_APPEND / O_TRUNC */
int part_cp(char *src, char *dst, char *mode) {
	int flag =0;
	char buf[256];
	FILE *fsrc, *fdst;

	fsrc = fopen(src, "r");
	if (fsrc == NULL)
		return 0;
	fdst = fopen(dst, mode);
	if (fdst == NULL) {
		fclose(fsrc);
		return 0;
	}
	while (fgets(buf, 256, fsrc)!=NULL) {
		if (flag==1&&!strcmp(buf, "--\n")) {
			fputs(buf, fdst);
			break;
		}
		if (flag==0&&(!strncmp(buf+2, "ÐÅÈË: ", 6) ||!strncmp(buf,
				"[1;41;33m·¢ÐÅÈË: ", 18))) {
			fputs(buf, fdst);
			continue;
		}
		if (flag==0&&(buf[0]=='\0'||buf[0]=='\n'/*||!strncmp(buf+2,"ÐÅÈË: ",6)*/
		|| !strncmp(buf, "±ê  Ìâ: ", 8)||!strncmp(buf, "·¢ÐÅÕ¾: ", 8)
		/*|| !strncmp(buf,"[1;41;33m·¢ÐÅÈË: ",18)*/))
			continue;
		flag =1;
		fputs(buf, fdst);
	}
	fclose(fdst);
	fclose(fsrc);
	return 1;
}

// Copies file 'src' to 'dst'. Returns 0 on success, -1 on error.
// 'mode' specifies the open mode of 'dst', usually O_APPEND or O_EXCL.
int f_cp(const char *src, const char *dst, int mode)
{
	int fsrc, fdst, ret = 0;
	if ((fsrc = open(src, O_RDONLY)) == -1)
		return -1;
	if ((fdst = open(dst, O_WRONLY | O_CREAT | mode, 0600)) >= 0) {
		char buf[BUFSIZ];
		do {
			ret = read(fsrc, buf, BUFSIZ);
			if (ret <= 0)
				break;
		} while (write(fdst, buf, ret) > 0);
		close(fdst);
	}
	close(fsrc);
	return ret;
}

// Makes a hard link 'dst' pointing to 'src'.
// If hard link cannot be created, try to copy 'src' to 'dst'.
// Returns 0 on success, -1 on error.
int f_ln(const char *src, const char *dst)
{
	if (link(src, dst) != 0) {
		if (errno != EEXIST)
			return f_cp(src, dst, O_EXCL);
	}
	return 0;
}

int valid_fname(char *str) {
	char ch;
	while ((ch = *str++) != '\0') {
		if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch
				>='0' && ch <= '9') || ch=='-' || ch=='_') {
			;
		} else {
			return 0;
		}
	}
	return 1;
}

/*
 Commented by Erebus 2004-11-08 
 rm file and folder
 */
int f_rm(char *fpath) {
	struct stat st;
	if (stat(fpath, &st)) //statÎ´ÄÜ³É¹¦
		return -1;

	if (!S_ISDIR(st.st_mode)) //²»ÊÇÄ¿Â¼,ÔòÉ¾³ý´ËÎÄ¼þ
		return unlink(fpath);

	return rm_dir(fpath); //É¾³ýÄ¿Â¼
}

/*
 Commented by Erebus 2004-11-08
 rm folder
 */

static int rm_dir(char *fpath) {
	struct stat st;
	DIR * dirp;
	struct dirent *de;
	char buf[256], *fname;
	if (!(dirp = opendir(fpath)))
		return -1;

	for (fname = buf; (*fname = *fpath) != '\0'; fname++, fpath++)
		;

	*fname++ = '/';

	readdir(dirp);
	readdir(dirp);

	while ((de = readdir(dirp)) != NULL) {
		fpath = de->d_name;
		if (*fpath) {
			strcpy(fname, fpath);
			if (!stat(buf, &st)) {
				if (S_ISDIR(st.st_mode))
					rm_dir(buf);
				else
					unlink(buf);
			}
		}
	}
	closedir(dirp);

	*--fname = '\0';
	return rmdir(buf);
}
