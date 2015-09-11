#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
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
			(void) file_lock_all(fd, FILE_UNLCK);
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

char *file_read_all(const char *file)
{
	if (!file)
		return NULL;

	int fd = open(file, O_RDONLY);
	if (fd < 0)
		return NULL;

	char *str = NULL;

	struct stat st;
	if (fstat(fd, &st) == 0 && st.st_size >= 0) {
		str = malloc(st.st_size + 1);
		if (str) {
			file_read(fd, str, st.st_size);
			str[st.st_size] = '\0';
		}
	}

	close(fd);
	return str;
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

void file_close_all(void)
{
	int n = sysconf(_SC_OPEN_MAX);
	while (n)
		file_close(--n);
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

int rm_dir_callback(const char *fpath, const struct stat *sb, int type,
		struct FTW *ftwbuf)
{
	if (type == FTW_F || type == FTW_SL || type == FTW_SLN) {
		return unlink(fpath);
	} else if (type == FTW_D || type == FTW_DP) {
		return rmdir(fpath);
	} else {
		return -1;
	}
}

static int rm_dir(const char *fpath)
{
	if (!fpath)
		return -1;
	return nftw(fpath, rm_dir_callback, 20, FTW_DEPTH | FTW_PHYS);
}

int file_rm(const char *fpath)
{
	struct stat st;
	if (stat(fpath, &st))
		return -1;

	if (!S_ISDIR(st.st_mode))
		return unlink(fpath);

	return rm_dir(fpath);
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

int file_try_lock_all(int fd, file_lock_e type)
{
	struct flock lock = {
		.l_type = type,
		.l_start = 0,
		.l_whence = FILE_SET,
		.l_len = 0,
	};
	return fcntl(fd, F_SETLK, &lock);
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
