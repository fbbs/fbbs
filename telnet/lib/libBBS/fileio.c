/*
 * fileio.c		-- some stuff that file/io related
 *
 * of SEEDNetBBS generation 1 (libtool implement)
 *
 * Copyright (c) 1999, Edward Ping-Da Chuang <edwardc@edwardc.dhs.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * CVS: $Id: fileio.c 366 2007-05-12 16:35:51Z danielfree $
 */

#ifdef BBS
#include "bbs.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

char fileio_c[] = "$Id: fileio.c 366 2007-05-12 16:35:51Z danielfree $";
#define        BLK_SIZ         4096

static int rm_dir();

//½«ÏûÏ¢msg´æÈëµ½ÎÄ¼þfpathÖÐ,Ð´Ê±Ëø×¡ÎÄ¼þ,Ð´Íê½âËø
//****	ÎÄ¼þ²Ù×÷¾ùÊÇµ×²ãÎÄ¼þIO²Ù×÷,¼Ó¿ìËÙ¶È
void file_append(char *fpath, char *msg) {
	int fd;
	if ((fd = open(fpath, O_WRONLY | O_CREAT, 0644)) >= 0) {
		flock(fd, LOCK_EX);
		lseek(fd, 0, SEEK_END);
		write(fd, msg, strlen(msg));
		flock(fd, LOCK_UN);
		close(fd);
	}
}
//Èôfname´æÔÚ,ÇÒÎªÕý³£ÎÄ¼þ,·µ»ØÕæ
int dashf(char *fname) {
	struct stat st;
	return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}

//Èç¹ûfname´æÔÚ,ÇÒÎªÄ¿Â¼,·µ»ØÕæ
int dashd(char *fname) {
	struct stat st;
	return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
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

int f_cp(char *src, char *dst, int mode) {
	int fsrc, fdst, ret;
	ret = 0;

	if ((fsrc = open(src, O_RDONLY)) >= 0) {
		ret = -1;

		if ((fdst = open(dst, O_WRONLY | O_CREAT | mode, 0600)) >= 0) {
			char pool[BLK_SIZ];
			src = pool;
			do {
				ret = read(fsrc, src, BLK_SIZ);
				if (ret <= 0)
					break;
			} while (write(fdst, src, ret) > 0);
			close(fdst);
		}
		close(fsrc);
	}
	return ret;
}

int f_ln(src, dst)
char *src, *dst;
{
	int ret;

	if ((ret = link(src, dst))!=0) {
		if (errno != EEXIST)
		ret = f_cp(src, dst, O_EXCL);
	}
	return ret;
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

int touchfile(char *filename) {
	int fd;
	if ((fd = open(filename, O_RDWR | O_CREAT, 0600)) > 0)
		close(fd);

	return fd;
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

	for (fname = buf; *fname = *fpath; fname++, fpath++)
		;

	*fname++ = '/';

	readdir(dirp);
	readdir(dirp);

	while (de = readdir(dirp)) {
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
