/* bbstop.c -- compute the top login/stay/post */

#include "bbs.h"

enum {
	KEY_LOGINS = 1,
	KEY_POSTS = 2,
	KEY_STAY = 3,
	KEY_PERF = 4
};

int login_cmp(const void *left, const void *right)
{
	const struct userec *a = left;
	const struct userec *b = right;
    return (b->numlogins - a->numlogins);
}

int post_cmp(const void *left, const void *right)
{
	const struct userec *a = left;
	const struct userec *b = right;
    return (b->numposts - a->numposts);
}

int stay_cmp(const void *left, const void *right)
{
	const struct userec *a = left;
	const struct userec *b = right;
    return (b->stay - a->stay);
}

int perf_cmp(const void *left, const void *right)
{
	const struct userec *a = left;
	const struct userec *b = right;
    return (b->numlogins / 3 + b->numposts + b->stay / 3600)
			- (a->numlogins / 3 + a->numposts + a->stay / 3600);
}

int getvalue(struct userec *user, int mode)
{
	switch (mode) {
		case KEY_LOGINS:
			return user->numlogins;
		case KEY_POSTS:
			return user->numposts;
		case KEY_STAY:
			return user->stay / 3600;
		case KEY_PERF:
		default:
			return user->numlogins / 3 + user->numposts + user->stay / 3600;
	}
	return 0;
}

void top_info(const char *item, int num, struct userec *users, int mode)
{
	printf("名次 帐号         %6.6s 名次 帐号         %6.6s 名次 帐号         %6.6s\n"
			"==== ============ ====== ==== ============ ====== ==== ============ ======\n",
			item, item, item);
	int rows = (num + 2) / 3;
	int i, j, rank;
	for (i = 0; i < rows; i++) {
		for (j = 0; j < 3; j++) {
			rank = i + j * rows;
			if (rank < num)
				printf("[%2d] %-12.12s %6d ", rank + 1, users[rank].userid, 
						getvalue(users + rank, mode));
		}
		printf("\n");
	}
}

struct userec alluser[MAXUSERS]; // avoid exceeding default stack size limit

int main(int argc, char **argv)
{
    FILE *inf;
    int num = 0, mode = 0;
	char passwd_file[256];
	char *home_path;

	struct userec *user = alluser;
	struct userec *end = user + MAXUSERS;

    if (argc < 4 ) {
        printf("Usage: %s bbs_home num_top mode\nmode=(0All 1Logins 2Posts 3Stay 4Perf)\n", argv[0]);
        exit(1);
    }
    home_path = argv[1];
    sprintf(passwd_file, "%s/.PASSWDS", home_path);
    
    num = atoi(argv[2]);
    mode = atoi(argv[3]);
    if (mode > KEY_PERF || mode < KEY_LOGINS)
        mode = 0;
    if (num == 0)
		num = 20;

    inf = fopen(passwd_file, "rb");
    if (inf == NULL) { 
        printf("Sorry, the data is not ready.\n"); 
        exit(0);
	}

	for (user = alluser; user < end; user++) {
		if (fread(user, sizeof(*user), 1, inf) <= 0)
			break;
		if (!strcmp(user->userid, "guest")) {
			user--;
			continue;
		}
	}

	if (mode == KEY_LOGINS || mode == 0) {
		qsort(alluser, sizeof(alluser) / sizeof(*user), sizeof(*user), login_cmp);
		top_info("上站数", num, alluser, KEY_LOGINS);
    }
	if (mode == KEY_POSTS || mode == 0) {
		qsort(alluser, sizeof(alluser) / sizeof(*user), sizeof(*user), post_cmp);
		top_info("发文数", num, alluser, KEY_POSTS);
    }
	if (mode == KEY_STAY || mode == 0) {
		qsort(alluser, sizeof(alluser) / sizeof(*user), sizeof(*user), stay_cmp);
		top_info("总时数", num, alluser, KEY_STAY);
    }
	if (mode == KEY_PERF || mode == 0) {
		qsort(alluser, sizeof(alluser) / sizeof(*user), sizeof(*user), perf_cmp);
		top_info("总积分", num, alluser, KEY_PERF);
    }
	return 0;
}
