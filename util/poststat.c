/***************************************************************************
                          poststat.c  -  description
                             -------------------
    begin                : ËÄ  3ÔÂ 10 2005
    copyright            : (C) 2005 by SpiritRain
    email                : SpiritRain@citiz.net
	last modified        : ¶þ  4ÔÂ 26 2005
 ***************************************************************************/

/*
*   ²ÎÊý: poststat [dir] [type]
*   type: 
*   0 ±íÊ¾Í³¼ÆÈÕÊ®´ó
*   1 ±íÊ¾Í³¼ÆÖÜÎåÊ®´ó
*   2 ±íÊ¾Í³¼ÆÔÂÒ»°Ù´ó
*   3 ±íÊ¾Í³¼ÆÄêÒ»°ÙÎåÊ®´ó
*/

//#define DEBUG			1

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define HASHSIZE		2048
#define AUTHORSIZE		13
#define BOARDSIZE		18
#define TITLESIZE		62

char *myfile[4] = {"day", "week", "month", "year"};
int mytop[4] = {10, 50, 100, 200};
char *mytitle[4] = {"ÈÕÊ®", "ÖÜÎåÊ®", "ÔÂÒ»°Ù", "Äê¶È¶þ°Ù"};


time_t now;

/* 105 bytes */
struct postrec {
    char author[AUTHORSIZE];              /* author name */
    char board[BOARDSIZE];               /* board name */
    char title[TITLESIZE];               /* title name */
    int gid;
    time_t date;                  /* last post's date */
    int number;                   /* post number */
};

struct postlist {
    char author[AUTHORSIZE];              /* author name */
    char board[BOARDSIZE];               /* board name */
    char title[TITLESIZE];               /* title name */
    int gid;
    time_t date;                  /* last post's date */
    int number;                   /* post number */
	struct postlist  *bnext, *prev, *next;
}*bucket[HASHSIZE],*toppost;


int	hash(char *key) {
    int i, value = 0;

    for (i = 0; key[i] && i < TITLESIZE; i++)
        value += key[i] < 0 ? -key[i] : key[i];

    value = value % HASHSIZE;
    return value;
}


/* ---------------------------------- */
/* hash structure : array + link list */
/* ---------------------------------- */
static __inline__ void add_rec(struct postlist * rec, struct postlist * prev,struct postlist * next)
{
	next->prev = rec;
	rec->next = next;
	rec->prev = prev;
	prev->next = rec;
}
static __inline__ void del_rec(struct postlist * prev,
				  struct postlist * next)
{
	next->prev = prev;
	prev->next = next;
}


void swap(struct postlist *i, struct postlist * j){
	del_rec(i->prev, i->next);
	del_rec(j->prev, j->next);
	add_rec(i, j->prev, j->next);
	add_rec(j, i->prev, i->next);
}

void sort(struct postlist *rec)
{
	struct postlist *tmppost = rec->prev;

	while (rec->number > tmppost->number && tmppost != toppost)
	{
		tmppost = tmppost->prev;
	}
	del_rec(rec->prev, rec->next);
	add_rec(rec, tmppost, tmppost->next);
}
/*
//Á´±í²åÈëÊ²Ã´·½·¨±È½Ï¸ßÐ§°¡£¿Í·ÔÎÁË-_-b
//1.0°æÏÈÖ»ÊµÏÖÒ»¸öË³ÐòËÑË÷
//Ð§ÂÊÌ«²î£¬²»ÒªÁË
void sort_all(){
	struct postlist *prev = toppost->next, *post = prev->next, *next;
	int count = 0;
	while (post != toppost)
	{
		count++;
		next = post->next;
		prev = post->prev;
		while (prev->number < post->number && prev != toppost)
		{
			prev = prev->prev;
		}
		if (prev != post->prev)
		{
			del_rec(post->prev, post->next);
			add_rec(post, prev, prev->next);
		}
		post = next;
	}
}
*/


void search(struct postlist *rec) {
    struct postlist *tmppost;
	//struct postlist *s;
    int 	h;
	//int		found = 0;

    //¼ÆËãtitleµÄhash
	//h = hash(rec->title);
	h = rec->gid % HASHSIZE;
	//Èç¹ûbucketÎª¿Õ
	rec->bnext = NULL;
	if (bucket[h] == NULL)
	{
		//bucketµÄµÚÒ»ÏîÎªrecord
		bucket[h] = rec;
		//recordµÄÏÂÒ»¸öÎª¿Õ
		//rec¼ÓÈëtoppostµÄË«Á¬±íÖÐ
		add_rec(rec,toppost->prev, toppost );
		sort(rec);
	}else{
		//ÁÙÊ±±äÁ¿ÎªbucketµÄµÚÒ»¸ö
	    tmppost = bucket[h];
		//Èç¹ûtmppost·Ç¿Õ£¬²¢ÇÒtmppostºÍrecµÄ°æÃû²»Í¬»òtitle²»Í¬
		while (1) 
		{
			//ÕÒµ½ÁË
			if (tmppost->gid == rec->gid && !strcmp(tmppost->board, rec->board))
			{
				//Ôö¼Ópost number
				tmppost->number += rec->number;
				//È¡½ÏÍíµÄdate
				if (tmppost->date < rec->date){
					tmppost->date = rec->date;
				}
				else{
					tmppost->date = rec->date;
				}
				sort(tmppost);
				return;
			}
			//Ã»ÕÒµ½
			if (tmppost->bnext == NULL)
			{
				//ËÑË÷ÍêÁÐ±í£¬rec¼ÓÈëÁÐ±íµ×²¿
				tmppost->bnext = rec;
				add_rec(rec,toppost->prev, toppost );
				sort(rec);
				return;
			}
			else
			{
				//ËÑË÷ÏÂÒ»Ìõ¼ÇÂ¼
				tmppost = tmppost->bnext;
			}
		}
	}
}

//´ÓfnameÖÐ¶ÁÈ¡¼ÇÂ¼£¬²¢ÎªÃ¿¸ö¼ÇÂ¼ÔÚlistÖÐÕÒµ½ÊÊµ±µÄÎ»ÖÃ
void load_stat(char *fname, size_t size) {
    FILE *fp;
	struct postlist *post;
	int listsize = sizeof(struct postlist); 
	long count = 0;
    //´ò¿ªfname
	if( (fp = fopen(fname, "r")) != NULL) {
		//·ÖÅäÒ»¸öÄÚ´æ¿é
		post = (struct postlist *)malloc(listsize);
		post->prev = post;
		post->next = post;
		post->bnext = NULL;
		//´ÓÎÄ¼þÖÐ¶ÁÈ¡¼ÇÂ¼
		while (fread(post, sizeof(struct postrec), 1, fp) && count < size)
		{
			//ËÑË÷ÁÐ±í£¬½«¼ÇÂ¼¼ÓÈë
			search(post);
			//ÖØÐÂ·ÖÅäÒ»¸öÄÚ´æ¿é
			count++;
			post = (struct postlist *)malloc(listsize);
			post->prev = post;
			post->next = post;
			post->bnext = NULL;
		};
		//ÊÍ·Å¶àÓàµÄÄÚ´æ¿é
		free(post);
		//¹Ø±Õfp
        fclose(fp);
    }
}


void poststat(int mytype) {
	FILE *fp;
	char srcfile[64], dstfile[64], *p;
    struct postlist *tmppost;
    int i, top, count;


	//Ê®´óÍ³¼ÆÆªÊý
	top = mytop[mytype];
	//³õÊ¼»¯toppostÊý¾Ý½á¹¹
	//toppostÊÇÒ»¸ö¿ÕµÄlist
	toppost = malloc(sizeof(struct postlist));
	toppost->prev = toppost;
	toppost->next = toppost;
	toppost->bnext = NULL;
	toppost->number = 0;


	//Í³¼ÆÈÕÊ®´ó
	if (mytype == 0)
	{
#ifdef DEBUG
		sprintf(srcfile,  "tmp/post");
		sprintf(dstfile,  "tmp/post.old");
#else
		sprintf(srcfile,  "tmp/.post");
		sprintf(dstfile,  "tmp/.post.old");
#endif
		//ÒÆ³ý tmp/.post.old
		remove(dstfile);
		
		//½« tmp/.post ÖØÃüÃûÎª tmp/.post.old
		rename(srcfile, dstfile);
		
		//ÏÈ¶ÁÈ¡etc/posts/day.0
		sprintf(srcfile, "etc/posts/day.0");
		load_stat(srcfile, -1);
		
		//ÔÙ¶ÁÈ¡ tmp/.post.old
		load_stat(dstfile, -1);
	}
	//Í³¼ÆÖÜÎåÊ®´ó
	else if (mytype > 0 && mytype < 4)
	{
		//ÏÈ¶ÁÈ¡±¾ÖÜtop¼ÇÂ¼etc/posts/week.0
		sprintf(srcfile,  "etc/posts/%s.0", myfile[mytype]);
		load_stat(srcfile, -1);

		//ÔÙ¶ÁÈ¡±¾ÈÕ¼ÇÂ¼etc/posts/day.0
		sprintf(dstfile, "etc/posts/day.0");
		load_stat(dstfile, -1);
	}
	//Í³¼ÆÔÂÒ»°Ù´ó
	else
	{
		return;
	}

	//°ÑÊ®´óÐ´»Ø
	if( (fp = fopen(srcfile, "w"))!=NULL) {
		count = 0;
		tmppost = toppost->next;
		while (tmppost != toppost)
		{
			fwrite(tmppost, sizeof(struct postrec), 1, fp);
			tmppost = tmppost->next;
		}
		fclose(fp);
	}

    sprintf(srcfile, "etc/posts/%s", myfile[mytype]);
    if( (fp = fopen(srcfile, "w"))!=NULL ) {
        fprintf(fp, "                [1;34m-----[37m=====[41m ±¾%s´óÈÈÃÅ»°Ìâ [40m=====[34m-----[0m\n\n", mytitle[mytype]);

		tmppost = toppost->next;
        for (i = 0; i < top && tmppost != toppost; i++) {
            strcpy(dstfile, ctime(&tmppost->date));
            dstfile[20] = '\0';
            p = dstfile + 4;
            fprintf(fp,
                    "[1;37mµÚ[1;31m%3d[37m Ãû [37mÐÅÇø : [33m%-18s[37m"
                    "¡¼ [32m%s[37m¡½[36m%4d [37mÆª[35m%13.13s\n"
                    "     [37m±êÌâ : [1;44;37m%-60.60s[40m\n"
                    ,(int)(i+1),tmppost->board,p,tmppost->number, tmppost->author, tmppost->title);
    		tmppost = tmppost->next;
		}
        fclose(fp);
    }

    /* free statistics */
	do
	{
		tmppost = toppost->next;
		del_rec(toppost,tmppost->next);
		free(tmppost);
	}
	while  (tmppost != toppost);
	for (i = 0; i < HASHSIZE ; i++ )
	{
		bucket[i] = NULL;
	}
}

//É¾³ýÔ­ÏÈµÄ etc/posts/????.0
//½« etc/posts/day.0 ÖØÃüÃûÎª etc/posts/day.x
//Èç´ËÔò¿ªÊ¼ÖØÐÂ¼ÆËãÊ®´ó
void reset(int type)
{
	char srcfile[64];
	sprintf(srcfile, "etc/posts/%s.0",myfile[type]);
	remove(srcfile);
}


int	main(int argc,char ** argv) {
#ifdef DEBUG
	int i, j;
	char buf[64];
	time_t t;
#endif
    struct tm *today;

#ifndef DEBUG
	if (argc < 2) {
		printf("Usage:\t%s bbshome [day]\n", argv[0]);
		return (-1);
	}
#endif
	chdir(argv[1]);
	if (argc == 3) {
		poststat(atoi(argv[2]));
		return (0);
	}
	time(&now);
    today = localtime(&now);
#ifdef DEBUG
	printf("mon:%x\tday:%d\thour:%d\tmin:%d\n", today->tm_mon , today->tm_mday , today->tm_hour,today->tm_min);
#endif
/*
	for (i = 0 ; i < 24 ; i++ )
	{
		for (j = 0; j < 60 ; j+=15 )
		{
			sprintf(buf, "tmp/.post.71%d%d", i, j);
			rename(buf,"tmp/post");
			poststat(0);
	printf("mon:%x\tday:%d\thour:%d\tmin:%d\n", today->tm_mon , today->tm_mday , today->tm_hour,today->tm_min);
		}
	}
	*/
#ifdef DEBUG
	printf("mon:%x\tday:%d\thour:%d\tmin:%d\n", today->tm_mon , today->tm_mday , today->tm_hour,today->tm_min);
#endif


// Èç¹ûhourÊÇ0µã,ÔòÔÚ0µãµÄÇ°10·ÖÖÓµ÷ÓÃµ½main
    //
	//Éú³Éµ±ÈÕÊ®´ó
    poststat(0);
    today = localtime(&now);
//    today = localtime(&now);
	//Áè³¿0µãµÄÊ±ºò
	if (today->tm_hour == 0 && today->tm_min <10) {
		//Éú³É±¾ÖÜÎåÊ®´ó
		poststat(1);
		//Éú³É±¾ÔÂÒ»°Ù´ó
		poststat(2);
		//Éú³É±¾ÄêÒ»°ÙÎåÊ®´ó
		poststat(3);

		today = localtime(&now);
#ifdef DEBUG
		printf("mon:%x\tday:%d\thour:%d\tmin:%d\n", today->tm_mon , today->tm_mday , today->tm_hour,today->tm_min);
#endif
		//É¾³ý¾ÉµÄday.0ÈÕÖ¾ÎÄ¼þ£¬´ÓÐÂ¿ªÊ¼Í³¼ÆÃ¿ÈÕÊ®´ó
		reset(0);

        //Èç¹ûweek dayÊÇÖÜÈÕ,É¾³ýweek.0ÈÕÖ¾ÎÄ¼þ£¬´ÓÐÂ¿ªÊ¼Í³¼ÆÃ¿ÖÜÎåÊ®´ó
       if (today->tm_wday == 0){
			reset(1);
		}

        //Èç¹ûmonth dayÊÇ1ºÅ,É¾³ýmonth.0ÈÕÖ¾ÎÄ¼þ£¬´ÓÐÂ¿ªÊ¼Í³¼ÆÃ¿ÔÂÒ»°Ù´ó
        if (today->tm_mday == 1){
            reset(2);

		}
		
		//Èç¹ûÊÇ1ÔÂ1ºÅ£¬É¾³ýyear.0ÈÕÖ¾ÎÄ¼þ£¬´ÓÐÂ¿ªÊ¼Í³¼ÆÃ¿ÄêÒ»°ÙÎåÊ®´ó
	//	today = localtime(&now);
		if (today->tm_yday == 0)
		{
			reset(3);
		}
    }
#ifdef DEBUG
   time(&t);
   printf("time: %d\n", t - now);
#endif
   return 0;
	
}
