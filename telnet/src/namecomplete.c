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
 $Id: namecomplete.c 366 2007-05-12 16:35:51Z danielfree $
 */

#include "bbs.h"

//      ºê¶¨Òåtoupper(c),µ«ÓÐ¸º×÷ÓÃ,ÓÃÊ±×¢Òâ
#define chartoupper(c)  ((c >= 'a' && c <= 'z') ? c+'A'-'a' : c)

#define NUMLINES (t_lines - 4)

struct word {
	char *name;
	struct word *next;
};

struct word *toplev = NULL, *current = NULL;

//  ÊÍ·ÅÃû×ÖÁÐ±í¿Õ¼ä
void FreeNameList() {
	struct word *p, *temp;
	for (p = toplev; p != NULL; p = temp) {
		temp = p->next;
		free(p->name);
		free(p);
	}
}

//      Çå¿ÕNameListÄÚ´æ,½«toplev,current¸´ÖÃÎªNULL
void CreateNameList() {
	if (toplev)
		FreeNameList();
	toplev = NULL;
	current = NULL;
}

int SeekInNameList(char *name) {
	struct word *p;
	if (name == NULL || !strcmp(name, ""))
		return 0;
	for (p = toplev; p != NULL; p = p->next) {
		if (!strcasecmp(p->name, name))
			return 1;
	}
	return 0;
}

int AddNameList(char *name) {
	struct word *node;
	if (SeekInNameList(name))
		return 0;
	node = (struct word *) malloc(sizeof(struct word));
	node->next = NULL;
	node->name = (char *) malloc(strlen(name) + 1);
	strcpy(node->name, name);
	if (toplev == NULL) {
		toplev = node;
		current = node;
	} else {
		current->next = node;
		current = node;
	}
	return 1;
}

int NumInList(register struct word *list) {
	register int i;
	for (i = 0; list != NULL; i++, list = list->next)
		/* Null Statement */;
	return i;
}

//      ¶ÔNameList½øÐÐfptr²Ù×÷
void ApplyToNameList(int (*fptr) ()) {
	struct word *p;
	for (p = toplev; p != NULL; p = p->next)
		(*fptr)(p->name);
}

int chkstr(char *otag, char *tag, char *name) {
	char ch, *oname = name;
	while (*tag != '\0') {
		ch = *name++;
		if (*tag != chartoupper (ch))
			return 0;
		tag++;
	}

	if (*tag != '\0' && *name == '\0')
		strcpy(otag, oname);

	return 1;
}

struct word * GetSubList(register char *tag, register struct word *list) {
	struct word *wlist, *wcurr;
	char tagbuf[STRLEN];
	int n;
	wlist = NULL;
	wcurr = NULL;
	for (n = 0; tag[n] != '\0'; n++) {
		tagbuf[n] = chartoupper (tag[n]);
	}
	tagbuf[n] = '\0';
	while (list != NULL) {
		if (chkstr(tag, tagbuf, list->name)) {
			register struct word *node;
			node = (struct word *) malloc(sizeof(struct word));
			node->name = list->name;
			node->next = NULL;
			if (wlist)
				wcurr->next = node;
			else
				wlist = node;
			wcurr = node;
		}
		list = list->next;
	}
	return wlist;
}

void ClearSubList(struct word *list) {
	struct word *tmp_list;
	while (list) {
		tmp_list = list->next;
		free(list);
		list = tmp_list;
	}
}

int MaxLen(struct word *list, int count) {
	int len = strlen(list->name);
	while (list != NULL && count) {
		int t = strlen(list->name);
		if (t > len)
			len = t;
		list = list->next;
		count--;
	}
	return len;
}

int namecomplete(char *prompt, char *data) {
	char *temp;
	int ch;
	int count = 0;
	int clearbot = NA;
	struct word *cwlist, *morelist;
	int x, y;
	int origx, origy;

	if (prompt != NULL) {
		prints("%s", prompt);
		clrtoeol();
	}
	temp = data;

	if (toplev == NULL)
		AddNameList("");
	cwlist = GetSubList("", toplev);
	morelist = NULL;
	getyx(&y, &x);
	getyx(&origy, &origx);
	while ((ch = igetkey()) != EOF) {
		if (ch == '\n' || ch == '\r') {
			*temp = '\0';
			prints("\n");
			if (NumInList(cwlist) == 1)
				strcpy(data, cwlist->name);
			else { /*  °æÃæ ID Ñ¡ÔñµÄÒ»¸ö¾«È·Æ¥ÅäÎÊÌâ  period */
				struct word *list;
				for (list = cwlist; list != NULL; list = list->next) {
					if (!ci_strcmp(data, list->name)) {
						strcpy(data, list->name);
						break;
					} //if
				} //for
			} //else
			ClearSubList(cwlist);
			break;
		}
		if (ch == ' ' || ch == KEY_TAB) {
			int col, len, i;
			if (NumInList(cwlist) == 1) {
				strcpy(data, cwlist->name);
				move(y, x);
				prints("%s", data + count);
				count = strlen(data);
				temp = data + count;
				getyx(&y, &x);
				continue;
			}
			for (i = strlen(data); i && i < STRLEN; i++) {
				struct word *node;

				ch = cwlist->name[i];
				if (ch == '\0')
					break;
				for (node = cwlist; node; node = node->next) {
					if (toupper(ch) != toupper(node->name[i]))
						break;
				}
				if (node != NULL)
					break;
				*temp++ = ch;
				count++;
				*temp = '\0';
				node = GetSubList(data, cwlist);
				if (node == NULL) {
					temp--;
					*temp = '\0';
					count--;
					break;
				}
				ClearSubList(cwlist);
				cwlist = node;
				morelist = NULL;
				move(y, x);
				addch(ch);
				x++;
			} //for
			clearbot = YEA;
			col = 0;
			if (!morelist)
				morelist = cwlist;
			len = MaxLen(morelist, NUMLINES);
			move(origy + 1, 0);
			clrtobot();
			standout();
			printdash(" ÁÐ±í ");
			standend();
			while (len + col < 80) {
				int i;
				for (i = NUMLINES; (morelist) && (i > origy - 1); i--) {
					if (morelist->name[0] != '\0') {
						move(origy + 2 + (NUMLINES - i), col);
						prints("%s", morelist->name);
					} else {
						i++;
					}
					morelist = morelist->next;
				}
				col += len + 2;
				if (!morelist)
					break;
				len = MaxLen(morelist, NUMLINES);
			} //while
			if (morelist) {
				move(t_lines - 1, 0);
				prints("[1;44m-- »¹ÓÐ --                                                                     [m");
			}
			move(y, x);
			continue;
		}
		if (ch == '\177' || ch == '\010') {
			if (temp == data)
				continue;
			temp--;
			count--;
			*temp = '\0';
			ClearSubList(cwlist);
			cwlist = GetSubList(data, toplev);
			morelist = NULL;
			x--;
			move(y, x);
			addch(' ');
			move(y, x);
			continue;
		} //if
		if (count < STRLEN) {
			struct word *node;
			*temp++ = ch;
			count++;
			*temp = '\0';
			node = GetSubList(data, cwlist);
			if (node == NULL) {
				temp--;
				*temp = '\0';
				count--;
				continue;
			}
			ClearSubList(cwlist);
			cwlist = node;
			morelist = NULL;
			move(y, x);
			addch(ch);
			x++;
		}
	} // while
	if (ch == EOF)
		longjmp(byebye, -1);
	prints("\n");
	refresh();
	if (clearbot) {
		move(origy, 0);
		clrtobot();
	}
	if (*data) {
		move(origy, origx);
		prints("%s\n", data);
		/* for (x=1; x<500; x++);  delay */
	}
	return 0;
}

int UserMaxLen(char cwlist[][IDLEN + 1], int cwnum, int morenum, int count) {
	int len, max = 0;
	while (count-- > 0 && morenum < cwnum) {
		len = strlen(cwlist[morenum++]);
		if (len > max)
			max = len;
	}
	return max;
}

int UserSubArray(char cwbuf[][IDLEN + 1], char cwlist[][IDLEN + 1],
		int cwnum, int key, int pos) {
	int key2, num = 0;
	int n, ch;
	key = chartoupper (key);
	if (key >= 'A' && key <= 'Z') {
		key2 = key - 'A' + 'a';
	} else {
		key2 = key;
	}
	for (n = 0; n < cwnum; n++) {
		ch = cwlist[n][pos];
		if (ch == key || ch == key2) {
			strcpy(cwbuf[num++], cwlist[n]);
		}
	}
	return num;
}

int usercomplete(char *prompt, char *data) {
	char *u_namearray();
	char *cwbuf, *cwlist, *temp;
	int cwnum, x, y, origx, origy;
	int clearbot = NA, count = 0, morenum = 0;
	char ch;

	cwbuf = malloc(MAXUSERS * (IDLEN + 1));
	if (prompt != NULL) {
		prints("%s", prompt);
		clrtoeol();
	}
	temp = data;
	cwlist = u_namearray(cwbuf, &cwnum, "");
	getyx(&y, &x);
	getyx(&origy, &origx);
	while ((ch = igetkey()) != EOF) {
		if (ch == '\n' || ch == '\r') {
			int i;
			char *ptr;

			*temp = '\0';
			prints("\n");
			ptr = cwlist;
			for (i = 0; i < cwnum; i++) {
				if (ci_strncmp(data, ptr, IDLEN + 1) == 0)
					strcpy(data, ptr);
				ptr += IDLEN + 1;
			}
			/* if( cwnum == 1 ) strcpy( data, cwlist ); */
			break;
		} // if
		if (ch == ' ' || ch == KEY_TAB) {
			int col, len, i, j;
			int n;

			if (cwnum == 1) {
				strcpy(data, cwlist);
				move(y, x);
				prints("%s", data + count);
				count = strlen(data);
				temp = data + count;
				getyx(&y, &x);
				continue;
			}
			for (i = strlen(data); i && i < IDLEN; i++) {
				ch = cwlist[i];
				if (ch == '\0')
					break;
				for (j = 0; j < cwnum; j++) {
					if (toupper((cwlist + (IDLEN + 1) * j)[i]) != toupper(ch))
						break;
				}
				if (j != cwnum)
					break;
				*temp++ = ch;
				*temp = '\0';
				n = UserSubArray((void *)cwbuf, (void *)cwlist, cwnum, ch,
						count);
				if (n == 0) {
					temp--;
					*temp = '\0';
					break;
				}
				cwlist = cwbuf;
				count++;
				cwnum = n;
				morenum = 0;
				move(y, x);
				addch(ch);
				x++;
			}
			clearbot = YEA;
			col = 0;
			len = UserMaxLen((void *)cwlist, cwnum, morenum, NUMLINES);
			move(origy + 1, 0);
			clrtobot();
			printdash(" ËùÓÐÊ¹ÓÃÕßÁÐ±í ");
			while (len + col < 79) {
				int i;
				for (i = 0; morenum < cwnum && i < NUMLINES - origy + 1; i++) {
					char *tmpptr = cwlist + (IDLEN + 1) * morenum++;
					if (*tmpptr != '\0') { //by Eric
						move(origy + 2 + i, col);
						prints("%s ", tmpptr);
					} else
						i--;
				}
				col += len + 2;
				if (morenum >= cwnum)
					break;
				len = UserMaxLen((void *)cwlist, cwnum, morenum, NUMLINES);
			}
			if (morenum < cwnum) {
				move(t_lines - 1, 0);
				prints("[1;44m-- »¹ÓÐÊ¹ÓÃÕß --                                                               [m");
			} else {
				morenum = 0;
			}
			move(y, x);
			continue;
		}
		if (ch == '\177' || ch == '\010') {
			if (temp == data)
				continue;
			temp--;
			count--;
			*temp = '\0';
			cwlist = u_namearray(cwbuf, &cwnum, data);
			morenum = 0;
			x--;
			move(y, x);
			addch(' ');
			move(y, x);
			continue;
		}
		if (count < STRLEN) {
			int n;
			*temp++ = ch;
			*temp = '\0';
			n = UserSubArray((void *)cwbuf, (void *)cwlist, cwnum, ch,
					count);
			if (n == 0) {
				temp--;
				*temp = '\0';
				continue;
			}
			cwlist = cwbuf;
			count++;
			cwnum = n;
			morenum = 0;
			move(y, x);
			addch(ch);
			x++;
		}
	}
	free(cwbuf);
	if (ch == EOF)
		longjmp(byebye, -1);
	prints("\n");
	refresh();
	if (clearbot) {
		move(origy, 0);
		clrtobot();
	}
	if (*data) {
		move(origy, origx);
		prints("%s\n", data);
	}
	return 0;
}
