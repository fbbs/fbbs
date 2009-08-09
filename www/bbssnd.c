#include "libweb.h"

int bbssnd_main(void)
{
	parse_post_data();
	int bid = strtol(getparm("bid"), NULL, 10);
	struct boardheader *bp = getbcache2(bid);

	if (!loginok)
		return BBS_ELGNREQ;
	if (bp == NULL || !haspostperm(&currentuser, bp))
		return BBS_ENOBRD;
	if (bp->flag & BOARD_DIR_FLAG)
		return BBS_EINVAL;

	unsigned int fid;
	struct fileheader fh;
	char *f = getparm("f");
	bool reply = !(*f == '\0');
	if (reply) {
		fid = strtoul(f, NULL, 10);
		if (!bbscon_search(bp, fid, 0, &fh))
			return BBS_ENOFILE;
		if (fh.accessed[0] & FILE_NOREPLY)
			return BBS_EACCES;
	}

	char title[sizeof(fh.title)];
	char *t = getparm("title");
	if (*t == '\0')
		return BBS_EINVAL;
	else
		strlcpy(title, t, sizeof(title));
	ansi_filter(title, title);

// TODO: ...
#ifdef SPARC
		if(abs(time(0) - *(int*)(u_info->from+34))<6) { //modified from 36 to 34 for sparc solaris by roly 02.02.28
			*(int*)(u_info->from+34)=time(0); //modified from 36 to 34 for sparc solaris by roly 02.02.28
			return BBS_EPFREQ;
		}
		*(int*)(u_info->from+34)=time(0);//modified from 36 to 34 for sparc solaris by roly 02.02.28
#else
		if(abs(time(0) - *(int*)(u_info->from+36))<6) { //modified from 36 to 34 for sparc solaris by roly 02.02.28
			*(int*)(u_info->from+36)=time(0); //modified from 36 to 34 for sparc solaris by roly 02.02.28
			return BBS_EPFREQ;
		}
		*(int*)(u_info->from+36)=time(0);//modified from 36 to 34 for sparc solaris by roly 02.02.28
#endif
	if (post_article(&currentuser, bp, title, 
			getparm("text"), fromhost, reply ? &fh : NULL) < 0)
		return BBS_EINTNL;

	if (!junkboard(bp)) {
		currentuser.numposts++;
		save_user_data(&currentuser);
	}

	char buf[sizeof(fh.title) + sizeof(bp->filename)];
	snprintf(buf, sizeof(buf), "posted '%s' on %s", title, bp->filename);
	report(buf, currentuser.userid);

	snprintf(buf, sizeof(buf), "bbsdoc?board=%s", bp->filename);
	http_header();
	refreshto(1, buf);
	printf("</head>\n<body>发表成功，1秒钟后自动转到<a href='%s'>版面</a>\n"
			"</body>\n</html>\n", buf);
	return 0;
}

