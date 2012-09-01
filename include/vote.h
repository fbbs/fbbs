#ifndef FB_VOTE_H
#define FB_VOTE_H

enum {
	VOTE_YN = 1,
	VOTE_SINGLE = 2,
	VOTE_MULTI = 3,
	VOTE_VALUE = 4,
	VOTE_ASKING = 5
};

struct ballot {
	char uid[IDLEN+1]; /* 投票人       */
	unsigned int voted; /* 投票的内容   */
	char msg[3][STRLEN]; /* 建议事项     */
};

struct votebal {
	char userid[IDLEN+1];
	char title[STRLEN];
	char type;
	char items[32][38];
	int maxdays;
	int maxtkt;
	int totalitems;
	int opendate;
};

#endif // FB_VOTE_H
