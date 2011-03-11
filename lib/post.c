#include <stdio.h>
#include <stdbool.h>
#include "bbs.h"
#include "record.h"
#include "fbbs/convert.h"
#include "fbbs/post.h"
#include "fbbs/string.h"

/**
 * Creates a new file in specific location.
 * @param[in] dir The directory.
 * @param[in] pfx Prefix of the file.
 * @param[in, out] fname The resulting filename.
 * @param[in] size The size of fname.
 * @return Filename and stream on success, NULL on error.
 * @see ::date_to_fname.
 */
static FILE *get_fname(const char *dir, const char *pfx,
		char *fname, size_t size)
{
	if (dir == NULL || pfx == NULL)
		return NULL;
	const char c[] = "ZYXWVUTSRQPONMLKJIHGFEDCBA";
	int t = (int)time(NULL);
	int count = snprintf(fname, size, "%s%s%d. ", dir, pfx, t);
	if (count < 0 || count >= size)
		return NULL;
	int fd;
	for (int i = sizeof(c) - 2; i >= 0; ++i) {
		fname[count - 1] = c[i];
		if ((fd = open(fname, O_CREAT | O_RDWR | O_EXCL, 0644)) > 0) {
			FILE *fp = fdopen(fd, "w+");
			if (fp) {
				return fp;
			} else {
				close(fd);
				return NULL;
			}
		}
	}
	return NULL;
}

static int convert_to_file(char *buf, size_t len, void *arg)
{
	FILE *fp = arg;
	if (fwrite(buf, 1, len, fp) < len)
		return -1;
	return 0;
}

/**
 * Post an article.
 * @param pr The post request.
 * @return file id on success, -1 on error.
 */
unsigned int do_post_article(const post_request_t *pr)
{
	if (!pr || !pr->title || !pr->content || !pr->bp)
		return 0;

	bool anony = pr->anony && (pr->bp->flag & BOARD_ANONY_FLAG);
	const char *userid = NULL, *nick = NULL, *ip = pr->ip;
	if (anony) {
		userid = ANONYMOUS_ACCOUNT;
		nick = ANONYMOUS_NICK;
		ip = ANONYMOUS_SOURCE;
	} else if (pr->user) {
		userid = pr->user->userid;
		nick = pr->user->username;
	} else if (pr->autopost) {
		userid = pr->userid;
		nick = pr->nick;
	}
	if (!userid || !nick)
		return 0;

	char dir[HOMELEN];
	int idx = snprintf(dir, sizeof(dir), "boards/%s/", pr->bp->filename);
	const char *pfx = "M.";

	char fname[HOMELEN];
	FILE *fptr;
	if ((fptr = get_fname(dir, pfx, fname, sizeof(fname))) == NULL)
		return 0;

	fprintf(fptr, "发信人: %s (%s), 信区: %s\n标  题: %s\n发信站: %s (%s)\n\n",
			userid, nick, pr->bp->filename, pr->title, BBSNAME,
			getdatestring(time(NULL), DATE_ZH));

	if (pr->cp)
		convert(pr->cp, pr->content, 0, NULL, 0, convert_to_file, fptr);
	else
		fputs(pr->content, fptr);

	if (!anony && pr->sig > 0)
		add_signature(fptr, userid, pr->sig);
	else
		fputs("\n--", fptr);

	if (ip) {
		char buf[2];
		fseek(fptr, -1, SEEK_END);
		fread(buf, 1, 1, fptr);
		if (buf[0] != '\n')
			fputs("\n", fptr);

		fprintf(fptr, "\033[m\033[1;%2dm※ %s:·"BBSNAME" "BBSHOST
			"·HTTP [FROM: %s]\033[m\n", 31 + rand() % 7,
			pr->crosspost ? "转载" : "来源", ip);
	}

	fclose(fptr);
	valid_gbk_file(fname, '?');

	struct fileheader fh;
	memset(&fh, 0, sizeof(fh));	
	strlcpy(fh.filename, fname + idx, sizeof(fh.filename));
	strlcpy(fh.owner, userid, sizeof(fh.owner));
	strlcpy(fh.title, pr->title, sizeof(fh.title));

	if (pr->noreply)
		fh.accessed[0] |= FILE_NOREPLY;
	if (pr->mmark)
		fh.accessed[0] |= FILE_MARKED;

	// TODO: assure fid order in .DIR
	fh.id = get_nextid2(pr->bp);
	if (pr->o_fp) { // reply
		fh.reid = pr->o_fp->id;
		fh.gid = pr->o_fp->gid;
	} else {
		fh.reid = fh.id;
		fh.gid = fh.id;
	}

	setwbdir(dir, pr->bp->filename);
	append_record(dir, &fh, sizeof(fh));
	updatelastpost(pr->bp->filename);

	if (!pr->autopost) {
		brc_fcgi_init(userid, pr->bp->filename);
		brc_addlist(fh.filename);
		brc_update(userid, pr->bp->filename);
	}

	return fh.id;
}
