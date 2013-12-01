#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "func.h"
#include "fbbs/fileio.h"

/**
 * 向文件尾部追加写入
 * @param[in] file 文件名
 * @param[in] msg 字符串
 * @return 成功返回写入字节数, 否则-1
 */
int file_append(const char *file, const char *msg)
{
	if (!file || !msg)
		return -1;
	int ret = -1;
	int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd != -1) {
		if (file_lock_all(fd, FILE_WRLCK) == 0) {
			ret = file_write(fd, msg, strlen(msg));
			file_lock_all(fd, FILE_UNLCK);
		}
		close(fd);
	}
	return ret;
}

/**
 * 从文件读取
 * @param[in] fd 文件描述符
 * @param[out] buf 缓冲区
 * @param[in] size 缓冲区大小
 * @return 成功返回读取字节数，否则返回-1
 */
int file_read(int fd, void *buf, size_t size)
{
	if (!buf)
		return -1;
	int res;
	do {
		res = read(fd, buf, size);
		if (res >= 0 || errno != EINTR)
			return res;
	} while (1);
}

/**
 * 向文件写入
 * @param[in] fd 文件描述符
 * @param[in] buf 缓冲区
 * @param[in] size 要写入的字节数
 * @return 成功返回写入字节数, 否则-1
 */
int file_write(int fd, const void *buf, size_t size)
{
	if (!buf)
		return -1;
	int res;
	do {
		res = write(fd, buf, size);
		if (res >= 0 || errno != EINTR)
			return res;
	} while (1);
}

/**
 * 关闭文件
 * @param[in] fd 文件描述符
 * @return 成功返回0, 否则-1.
 */
int file_close(int fd)
{
	int ret = close(fd);
	if (ret < 0 && errno == EINTR)
		return 0;
	return ret;
}

/**
 * 改变文件大小
 * @param[in] fd 文件描述符
 * @param[in] size 文件的新长度
 * @return 成功返回0, 否则-1
 */
int file_truncate(int fd, off_t size)
{
	while (ftruncate(fd, size) < 0) {
		if (errno != EINTR)
			return -1;
	}
	return 0;
}

/**
 * 检测文件是否存在且为普通文件
 * @param[in] file 文件名
 * @return 文件存在且为普通文件返回true, 否则false
 */
bool dashf(const char *file)
{
	if (!file)
		return false;
	struct stat st;
	return (stat(file, &st) == 0 && S_ISREG(st.st_mode));
}

/**
 * 检测目录是否存在
 * @param[in] file 目录名
 * @return 文件存在且为目录返回true, 否则false
 */
bool dashd(const char *file)
{
	if (!file)
		return false;
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
		//% "信人: "
		if (flag==0&&(!strncmp(buf+2, "\xd0\xc5\xc8\xcb: ", 6) ||!strncmp(buf,
				//% "\033[1;41;33m发信人: "
				"\033[1;41;33m\xb7\xa2\xd0\xc5\xc8\xcb: ", 18))) {
			fputs(buf, fdst);
			continue;
		}
		if (flag==0&&(buf[0]=='\0'||buf[0]=='\n'
		//% "标  题: " "发信站: "
		|| !strncmp(buf, "\xb1\xea  \xcc\xe2: ", 8)||!strncmp(buf, "\xb7\xa2\xd0\xc5\xd5\xbe: ", 8)
		))
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
		} while (file_write(fdst, buf, ret) > 0);
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

/*
 Commented by Erebus 2004-11-08 
 rm file and folder
 */
int f_rm(char *fpath) {
	struct stat st;
	if (stat(fpath, &st)) //stat未能成功
		return -1;

	if (!S_ISDIR(st.st_mode)) //不是目录,则删除此文件
		return unlink(fpath);

	return rm_dir(fpath); //删除目录
}

/**
 * 锁定/解锁文件
 * @param[in] fd 文件描述符
 * @param[in] type 锁的类型
 * @param[in] offset 锁的起始偏移量
 * @param[in] whence 偏移量的基准
 * @param[in] len 锁的长度
 * @return 成功返回0, 否则-1 
 */
int file_lock(int fd, file_lock_e type, off_t offset, file_whence_e whence,
		off_t len)
{
	struct flock lock = {
		.l_type = type,
		.l_start = offset,
		.l_whence = whence,
		.l_len = len,
	};
	return fcntl(fd, F_SETLKW, &lock);
}

/**
 * 锁定/解锁整个文件
 * @param[in] fd 文件描述符
 * @param[in] type 锁的类型
 * @return 成功返回0, 否则-1
 */
int file_lock_all(int fd, file_lock_e type)
{
	return file_lock(fd, type, 0, FILE_SET, 0);
}

/**
 * 生成一个临时文件的文件名
 * @param[out] file 文件名
 * @param[in] size 文件名最大长度
 * @param[in] prefix 前缀，将会出现在文件名中
 * @param[in] num 一个数字，将会出现在文件名中
 * @return 文件名的长度
 * @note 这个函数不应直接调用
 * @see file_temp_name
 */
int file_temporary_name(char *file, size_t size, const char *prefix, int num)
{
	if (!file || !prefix)
		return 0;
	return snprintf(file, size, "temp/%d-%s-%d", getpid(), prefix, num);
}
