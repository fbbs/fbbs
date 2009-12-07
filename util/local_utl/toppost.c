#include "bbs.h"

#define BASEPATH BBSHOME"/etc/posts"

enum {
	BOARD_LEN = 18, TITLE_LEN = 62, OWNER_LEN = 16,
	RESERVE_HOURS = 72, MAX_RECORDS = 4000,
	DAY = 0, WEEK = 1, MONTH = 2, YEAR = 3,
};

typedef struct top_t {
	unsigned int gid;
	char board[BOARD_LEN];
	char title[TITLE_LEN];
	char owner[OWNER_LEN];
	int count;
	time_t last;
} top_t;

const char *files[] = { "day", "week", "month", "year" };
const int limits[] = { 10, 50, 100, 200 };
const char *titles[] = { "日十", "周五十", "月一百", "年度二百" };

unsigned int top_hash(const char *key, unsigned int *klen)
{
	const top_t *top = (const top_t *)key;
	unsigned int hash = top->gid;
	unsigned int len = HASH_KEY_STRING;
	hash += hash_func_default(top->board, &len);
	*klen = sizeof(top->gid) + len;
	return hash;
}

void *top_alloc(void)
{
	static int max = 0;
	static int used = 0;
	static top_t *pool = NULL;
	if (used >= max) {
		if (!max)
			max = MAX_RECORDS;
		else
			max *= 2;
		pool = malloc(sizeof(top_t) * max);
		used = 0;
	}
	return pool + used++;
}

void load_stat(hash_t *ht, int type)
{
	top_t buf;
	char file[HOMELEN];
	sprintf(file, BASEPATH"/%s.0", files[type]);
	FILE *fp = fopen(file, "r");
	if (fp) {
		while (fread(&buf, sizeof(buf), 1, fp) == 1) {
			top_t *top = top_alloc();
			memcpy(top, &buf, sizeof(*top));
			if (type == DAY)
				top->count = 0;
			hash_set(ht, (char *)top, HASH_KEY_STRING, top);
		}
		fclose(fp);
	}
}

bool should_stat(const struct boardheader *bp)
{
	if ((bp->flag & (BOARD_DIR_FLAG | BOARD_POST_FLAG | BOARD_JUNK_FLAG))
			|| bp->level)
		return 0;
	return 1;
}

void process(hash_t *ht, const struct boardheader *bp)
{
	if (!should_stat(bp))
		return;
	time_t recent = time(NULL) - 24 * 60 * 60;
	char file[HOMELEN];
	setwbdir(file, bp->filename);
	FILE *fp = fopen(file, "r");
	if (!fp)
		return;
	struct fileheader fh;
	top_t top;
	fseek(fp, -sizeof(fh), SEEK_END);
	while (fread(&fh, sizeof(fh), 1, fp) == 1) {
		time_t last = (time_t)strtol(fh.filename + 2, NULL, 10);
		if (last < recent)
			break;
		top.gid = fh.gid;
		strlcpy(top.board, bp->filename, sizeof(top.board));
		top_t *entry = hash_get(ht, &top, HASH_KEY_STRING);
		if (entry) {
			entry->count++;
			if (last > entry->last)
				entry->last = last;
			if (fh.id == fh.gid) {
				strlcpy(entry->title, fh.title, sizeof(entry->title));
				strlcpy(entry->owner, fh.owner, sizeof(entry->owner));
			}
		} else {
			entry = top_alloc();
			entry->gid = fh.gid;
			strlcpy(entry->board, bp->filename, sizeof(entry->board));
			strlcpy(entry->title, fh.title, sizeof(entry->title));
			strlcpy(entry->owner, fh.owner, sizeof(entry->owner));
			entry->count = 1;
			entry->last = last;
			hash_set(ht, (char *)entry, HASH_KEY_STRING, entry);
		}
		if (ftell(fp) < 2 * sizeof(fh))
			break;
		fseek(fp, -2 * sizeof(fh), SEEK_CUR);
	}
	fclose(fp);
}

int cmp(const void *a, const void *b)
{
	const top_t **t1 = (const top_t **)a;
	const top_t **t2 = (const top_t **)b;
	return ((*t2)->count - (*t1)->count);
}

top_t **sort_stat(const hash_t *ht)
{
	top_t **tops = NULL;
	if (ht->count) {
		tops = malloc(sizeof(top_t) * ht->count);
		if (!tops)
			return NULL;
		hash_iter_t *iter;
		top_t **ptr = tops;
		for (iter = hash_begin(ht); iter; iter = hash_next(iter)) {
			*ptr++ = (top_t *)(iter->entry->val);
		}
		qsort(tops, ht->count, sizeof(*tops), cmp);
	}
	return tops;
}

void print_stat(const hash_t *ht, top_t **tops, int type)
{
	char file[HOMELEN];
	sprintf(file, BASEPATH"/%s", files[type]);
	FILE *fp = fopen(file, "w+");
	fprintf(fp, "                \033[1;34m-----\033[37m=====\033[41m"
			" 本%s大热门话题 \033[40m=====\033[34m-----\033[m\n\n",
			titles[type]);
	top_t *top;
	int i;
	int limit = limits[type] < ht->count ? limits[type] : ht->count;
	char date[32];
	char title[sizeof(top->title)];
	for (i = 0; i < limit; ++i) {
		top = tops[i];
		strlcpy(date, ctime(&top->last) + 4, 16);
		fprintf(fp, "\033[1;37m第\033[31m%3u\033[37m 名 \033[37m信区 : \033[33m"
				"%-18s\033[37m〖 \033[32m%s\033[37m 〗\033[36m%4d \033[37m篇"
				"\033[33m%13.13s\n     \033[37m标题 : \033[1;44m%-60.60s"
				"\033[40m\n", i + 1, top->board, date, top->count, top->owner,
				ansi_filter(title, top->title));
	}
	fclose(fp);
}

void save_stat(const hash_t *ht, top_t **tops, int type)
{
	time_t now = time(NULL);
	time_t recent = now - RESERVE_HOURS * 60 * 60;
	char file[HOMELEN];
	sprintf(file, BASEPATH"%s.0", files[type]);
	FILE *fp = fopen(file, "w+");
	if (!fp)
		return;
	int i;
	top_t *top;
	int limit = ht->count < MAX_RECORDS ? ht->count : MAX_RECORDS;
	for (i = 0; i < limit; ++i) {
		top = tops[i];
		if ((type == DAY && top->last >= recent) || top->count)
			fwrite(top, sizeof(*top), 1, fp);
	}
	fclose(fp);
}

void remove_stat(int type)
{
	char file[HOMELEN];
	sprintf(file, BASEPATH"/%s.0", files[type]);
	unlink(file);
}

void merge_stat(const hash_t *day, int type)
{
	hash_t stat;
	hash_create(&stat, 0, top_hash);
	load_stat(&stat, type);

	hash_iter_t *iter;
	top_t *entry;
	const top_t *top;
	for (iter = hash_begin(day); iter; iter = hash_next(iter)) {
		top = (const top_t *)(iter->entry->key);
		entry = hash_get(&stat, top, HASH_KEY_STRING);
		if (entry) {
			entry->count += top->count;
			entry->last = top->last;
		} else {
			if (top->count)
				hash_set(&stat, (char *)top, HASH_KEY_STRING, top);
		}
	}
	
	top_t **tops = sort_stat(&stat);
	if (tops) {
		print_stat(&stat, tops, type);
		save_stat(&stat, tops, type);
	}
}

int main(int argc, char **argv)
{
	chdir(BBSHOME);

	hash_t stat;
	hash_create(&stat, 0, top_hash);
	load_stat(&stat, DAY);

	FILE *fp = fopen(BBSHOME"/.BOARDS", "r");
	if (fp) {
		struct boardheader board;
		while (fread(&board, sizeof(board), 1, fp) == 1) {
			if (board.filename[0] != '\0')
				process(&stat, &board);
		}
		fclose(fp);
	} else {
		return EXIT_FAILURE;
	}

	top_t **tops = sort_stat(&stat);
	if (tops) {
		print_stat(&stat, tops, DAY);
		save_stat(&stat, tops, DAY);
	}

	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	if (t->tm_hour == 3 && t->tm_min < 10) {
		merge_stat(&stat, WEEK);
		merge_stat(&stat, MONTH);
		merge_stat(&stat, YEAR);
		if (t->tm_wday == 0)
			remove_stat(WEEK);
		if (t->tm_mday == 1)
			remove_stat(MONTH);
		if (t->tm_yday == 0)
			remove_stat(YEAR);
	}
	return EXIT_SUCCESS;
}

