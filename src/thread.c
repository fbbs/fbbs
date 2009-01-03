/*
 Pirate Bulletin Board System
 Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
 Eagles Bulletin Board System
 Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
 Guy Vega, gtvega@seabass.st.usm.edu
 Dominic Tynes, dbtynes@seabass.st.usm.edu
 Firebird Bulletin Board System
 Copyright (C) 1996, Hsien-Tsung Chang, Smallpig.bbs@bbs.cs.ccu.edu.tw
 Peng Piaw Foong, ppfoong@csie.ncu.edu.tw
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 1, or (at your option)
 any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 */
/*
 $Id: thread.c 2 2005-07-14 15:06:08Z root $
 */

#include "bbs.h"

char tname[STRLEN]; //.THREAD文件名,暂存中间结果
char fname[STRLEN]; //.DIR2文件,暂存中间结果

struct postnode { //记录同一主题的不同post
	int num;
	struct postnode *next;
};

struct titlenode { //每个titlenode记录不同主题
	char *title;
	struct titlenode *next;
	struct postnode *post;
};

struct titlenode toptitle; //同主题方式的头结点

// 释放所有动态分配的内存空间
int FreeTitleMem() {
	struct titlenode *t=toptitle.next;
	struct postnode *p=NULL;

	while (t !=NULL) {
		p = t->post;
		while (p!=NULL) {
			t->post = p->next;
			free(p); //释放每个post所占用的空间
			p= t->post;
		}
		toptitle.next=t->next;
		free(t->title); //释放标题字符串所占用的空间		
		free(t); //释放标题链表本身所占用的空间
		t = toptitle.next;
	}

	return 0;
}

//将第num个记录post加入到同主题列表中
//返回值现无意义
int thread(struct fileheader *post, int num) {
	struct titlenode *tmp;
	char *ntitle;
	tmp = &toptitle;

	//取得post的标题
	if (post->title[0] == 'R' && post->title[1] == 'e' && post->title[2]
			== ':') {
		ntitle = post->title + 4; //re文从第四个字符开始算起
	} else {
		ntitle = post->title;
	}

	while (1) {
		if (tmp->next == NULL) {
			struct titlenode *titletmp;
			titletmp
					= (struct titlenode *) malloc(sizeof(struct titlenode));
			titletmp->title = malloc(sizeof(char) * (strlen(ntitle) + 1));
			strcpy(titletmp->title, ntitle);
			titletmp->post = NULL;
			titletmp->next = NULL;
			tmp->next = titletmp;
		}
		if (!strcmp(tmp->next->title, ntitle)) { //找到其所属主题
			struct postnode *tmppost, *first = tmp->next->post;
			if (first == NULL) {
				tmppost
						= (struct postnode *) malloc(sizeof(struct postnode));
				tmppost->num = num;
				tmppost->next = NULL;
				tmp->next->post = tmppost;
				return 1;
			}
			while (1) {
				if (first->next == NULL) {
					tmppost
							=(struct postnode *)malloc(sizeof(struct postnode));
					tmppost->num = num;
					tmppost->next = NULL;
					first->next = tmppost;
					return 2;
				}
				first = first->next;
			}//while(1)
		} else {
			tmp = tmp->next; //tmp此时的主题非ntitle,接着找
		} //else
	} //while(1)

	return 0;
}
//访问toptitle中所有post记录,从fname中记取记录,修改后保存到tname中
int visit_all() {
	struct titlenode *tmp;
	struct fileheader post;
	int i = 0;
	tmp = toptitle.next;

	while (tmp) {
		struct postnode *tmppost;
		i++;
		tmppost = tmp->post;
		while (tmppost) {
			get_record(fname, &post, sizeof(post), tmppost->num);
			//added by iamfat 美观同主题 2002.3.11
			if (!tmppost->next) { //最后一项,供置底时使用?
				post.accessed[1]|=FILE_LASTONE;
			}
			append_record(tname, &post, sizeof(post));
			tmppost = tmppost->next;
		} //while (tmppost)
		tmp = tmp->next;
	}//while(tmp)

	return 0;
}

int main(int argc, char *argv[]) {
	FILE *tf;
	int i = 0;
	struct fileheader post;
	char dname[STRLEN];
	char buf[256];
	struct stat st1, st2;
	sprintf(dname, "boards/%s/%s", argv[1], DOT_DIR); //.DIR文件
	sprintf(fname, "boards/%s/%s2", argv[1], DOT_DIR); //.DIR2文件
	sprintf(tname, "boards/%s/%s", argv[1], THREAD_DIR);//.THREAD文件

	if (stat(dname, &st1) == -1) {
		return 1;
	}
	if (stat(tname, &st2) != -1) {
		if (st2.st_mtime >= st1.st_mtime)
			return 2;
	}
	unlink(tname);
	sprintf(buf, "cp %s %s", dname, fname);
	system(buf);

	if ((tf = fopen(fname, "rb")) == NULL) {
		printf(".DIR can't open...");
		return 3;
	}
	toptitle.next = NULL;
	toptitle.post = NULL;
	while (1) {
		i++;
		if (fread(&post, sizeof(post), 1, tf) <= 0)
			break;
		thread(&post, i);
	}
	visit_all();
	fclose(tf);
	FreeTitleMem();
	unlink(fname);

	return 0;
}
