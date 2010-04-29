#include <stdio.h>
#include <stdbool.h>
#include "bbs.h"
#include "post.h"

/**
 * Creates a new file in specific location.
 * @param[in] dir The directory.
 * @param[in] pfx Prefix of the file.
 * @param[in, out] fname The resulting filename.
 * @param[size] The size of fname.
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
		if ((fd = open(fname, O_CREAT | O_WRONLY | O_EXCL, 0644)) > 0) {
			FILE *fp = fdopen(fd, "w");
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

/**
 * Post an article.
 * @param pr The post request.
 * @return 0 on success, -1 on error.
 */
int do_post_article(const post_request_t *pr)
{
	if (!pr || !pr->title || !pr->content || !pr->bp)
		return -1;

	const char *userid = NULL, *nick = NULL;
	if (pr->user) {
		userid = pr->user->userid;
		nick = pr->user->username;
	} else if (pr->autopost) {
		userid = pr->userid;
		nick = pr->nick;
	}
	if (!userid || !nick)
		return -1;

	char dir[HOMELEN];
	int idx = snprintf(dir, sizeof(dir), "boards/%s/", pr->bp->filename);
	const char *pfx = "M.";

	char fname[HOMELEN];
	FILE *fptr;
	if ((fptr = get_fname(dir, pfx, fname, sizeof(fname))) == NULL)
		return -1;

	fprintf(fptr, "发信人: %s (%s), 信区: %s\n标  题: %s\n发信站: %s (%s)\n\n",
			userid, nick, pr->bp->filename, pr->title, BBSNAME,
			getdatestring(time(NULL), DATE_ZH));

	fputs(pr->content, fptr);

	if (pr->sig > 0)
		add_signature(fptr, userid, pr->sig);

	if (pr->ip) {
		fprintf(fptr, "\n\033[m\033[1;%2dm※ %s:·"BBSNAME" "BBSHOST
			"·HTTP [FROM: %-.20s]\033[m\n", 31 + rand() % 7,
			pr->crosspost ? "转载" : "来源", pr->ip);
	}

	fclose(fptr);

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

	return 0;
}
