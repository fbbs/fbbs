#include "bbs.h"
#include "hash.h"
#include "fbbs/helper.h"
#include "fbbs/pool.h"
#include "fbbs/post.h"
#include "fbbs/string.h"

#define BASEPATH BBSHOME"/etc/posts"

typedef uchar_t bitset_t;

static bitset_t *bitset_init(uint_t size)
{
	if (!size)
		return NULL;
	uint_t len = (size + 7) / 8;

	bitset_t *bs = malloc(len);
	if (bs)
		memset(bs, 0, len);
	return bs;
}

static void bitset_set(bitset_t *bs, uint_t pos, bool set)
{
	uchar_t *c = bs + pos / 8, bit = 1U << (pos % 8);
	if (set)
		*c |= bit;
	else
		*c &= bit;
}

static bool bitset_test(const bitset_t *bs, uint_t pos)
{
	return bs[pos / 8] & (1U << (pos % 8));
}

enum {
	PER_BOARD_LIMIT = 1,
	BACKUP_FACTOR = 5,
	BOARD_LEN = 18,
	OWNER_LEN = 16,
};

typedef struct {
	const char *file;
	const char *descr;
	int limit;
	int days;
	bool per_board_limit;
} config_t;

typedef struct {
	uint_t count;
	fb_time_t last;
} topic_stat_short_t;

typedef struct {
	pool_t *pool;
	config_t *config;
	topic_stat_t *topics;
	const bitset_t *bs;
	fb_time_t stamp;
	hash_t hash;
	uint_t size;
} stat_t;

static char (*bnames)[BOARD_NAME_LEN + 1];

static record_callback_e hot_topics_stat(post_index_t *pi, void *args)
{
	stat_t *stat = args;

	if (pi->stamp && pi->stamp <= stat->stamp)
		return RECORD_CALLBACK_BREAK;
	if (!pi->stamp)
		return RECORD_CALLBACK_CONTINUE;

	if (!bitset_test(stat->bs, pi->bid - 1)
			|| (pi->flag & POST_FLAG_DELETED))
		return RECORD_CALLBACK_CONTINUE;

	post_id_t tid = pi->id - pi->tid_delta;
	topic_stat_short_t *tss = hash_get(&stat->hash, &tid, sizeof(tid));
	if (tss) {
		++tss->count;
	} else {
		post_id_t *key = pool_alloc(stat->pool, sizeof(*key));
		*key = tid;
		tss = pool_alloc(stat->pool, sizeof(*tss));
		tss->count = 1;
		tss->last = pi->stamp;
		hash_set(&stat->hash, (char *) key, sizeof(*key), tss);
	}

	if (pi->id == tid && (stat->size < stat->config->limit
			|| tss->count > stat->topics[stat->config->limit - 1].count)) {
		int pos = stat->size ? stat->size - 1 : 0;
		if (stat->size) {
			while (pos >= 0 && tss->count > stat->topics[pos].count)
				--pos;
			++pos;
		}

		if (stat->size < stat->config->limit) {
			memmove(stat->topics + pos + 1, stat->topics + pos,
					(stat->size - pos) * sizeof(*stat->topics));
			++stat->size;
		} else {
			memmove(stat->topics + pos + 1, stat->topics + pos,
					(stat->size - pos - 1) * sizeof(*stat->topics));
		}

		topic_stat_t *ts = stat->topics + pos;
		ts->count = tss->count;
		ts->last = tss->last;
		ts->tid = tid;
		ts->bid = pi->bid;
		strlcpy(ts->utf8_title, pi->utf8_title, sizeof(ts->utf8_title));
		strlcpy(ts->owner, pi->owner, sizeof(ts->owner));
		strlcpy(ts->bname, bnames[pi->bid - 1], sizeof(ts->bname));
	}
	return RECORD_CALLBACK_CONTINUE;
}

static bitset_t *board_bitset;

static bitset_t *board_init(void)
{
	db_res_t *res = db_query("SELECT id, perm, flag, name FROM boards");
	if (!res)
		exit(EXIT_FAILURE);

	int max = 0;
	for (int i = db_res_rows(res) - 1; i >= 0; --i) {
		int bid = db_get_integer(res, i, 0);
		if (bid > max)
			max = bid;
	}

	board_bitset = bitset_init(max);
	bnames = malloc((max - 1) * sizeof(*bnames));

	for (int i = db_res_rows(res) - 1; i >= 0; --i) {
		int bid = db_get_integer(res, i, 0);
		int perm = db_get_integer(res, i, 1);
		int flag = db_get_integer(res, i, 2);
		const char *name = db_get_value(res, i, 3);
		strlcpy(bnames[bid - 1], name, sizeof(bnames[0]));
		if (!(flag & (BOARD_DIR_FLAG | BOARD_POST_FLAG | BOARD_JUNK_FLAG))
				&& !perm) {
			bitset_set(board_bitset, bid - 1, true);
		}
	}

	db_clear(res);
	return board_bitset;
}

static int topic_sort(const void *p1, const void *p2)
{
	const topic_stat_t *s1 = p1, *s2 = p2;
	if (s1->count == s2->count)
		return s2->last - s1->last;
	return s2->count - s1->count;
}

static void print_topic(FILE *fp, FILE *fp2, const topic_stat_t *topic,
		int rank)
{
	char time[32];
	strlcpy(time, fb_ctime(&topic->last) + 4, 16);

	GBK_BUFFER(title, POST_TITLE_CCHARS);
	convert_u2g(topic->utf8_title, gbk_title);
	ansi_filter(gbk_title, gbk_title);

	//% 第 %2d 名 信区 : %-18s〖 %s 〗%4d 篇 %s\n 标题 : %s\n"
	fprintf(fp, "\033[1;37m\xb5\xda \033[31m%2d\033[37m \xc3\xfb "
			"\033[37m\xd0\xc5\xc7\xf8 : \033[33m%-18s"
			"\033[37m\xa1\xbc \033[32m%s\033[37m \xa1\xbd"
			"\033[36m%4u \033[37m\xc6\xaa\033[33m%13.13s\n"
			"     \033[37m\xb1\xea\xcc\xe2 : \033[1;44m%-60.60s\033[40m\n",
			rank, topic->bname, time, topic->count, topic->owner,
			gbk_title);

	if (fp2)
		fwrite(topic, sizeof(*topic), 1, fp2);
}

static void print_header(FILE *fp, const char *descr)
{
	fprintf(fp, "                \033[1;34m-----\033[37m=====\033[41m"
			//% 本%s大热门话题
			" \xb1\xbe%s\xb4\xf3\xc8\xc8\xc3\xc5\xbb\xb0\xcc\xe2"
			" \033[40m=====\033[34m-----\033[m\n\n", descr);
}

static void print_stat(stat_t *stat)
{
	char file[HOMELEN];
	snprintf(file, sizeof(file), BASEPATH"/%s", stat->config->file);
	FILE *fp = fopen(file, "w");
	snprintf(file, sizeof(file), BASEPATH"/%s.data", stat->config->file);
	FILE *fp2 = fopen(file, "w");
	print_header(fp, stat->config->descr);

	for (int i = 0; i < stat->config->limit && i < stat->size; ++i) {
		print_topic(fp, NULL, stat->topics + i, i + 1);
	}
	fclose(fp2);
	fclose(fp);

	if (stat->config->per_board_limit) {
		snprintf(file, sizeof(file), BASEPATH"/%s_f", stat->config->file);
		fp = fopen(file, "w");
		snprintf(file, sizeof(file), BASEPATH"/%s_f.data", stat->config->file);
		fp2 = fopen(file, "w");

		print_header(fp, stat->config->descr);

		int rank = 0, duplicated = 0;
		for (int i = 0; i < stat->config->limit && i < stat->size
				&& rank <= stat->config->limit; ++i) {
			bool dup = false;
			for (int j = i - 1; j >= 0; --j) {
				if (stat->topics[j].bid == stat->topics[i].bid) {
					dup = true;
					break;
				}
			}

			if (dup) {
				++duplicated;
			} else {
				print_topic(fp, fp2, stat->topics + i, ++rank);
			}
		}
		fclose(fp2);
		if (duplicated) {
			//% 【有 %d 个主题因超出版面限制而被省略】
			fprintf(fp, "\033[1;30m  \xa1\xbe\xd3\xd0 %d \xb8\xf6"
					"\xd6\xf7\xcc\xe2\xd2\xf2\xb3\xac\xb3\xf6\xb0\xe6\xc3\xe6"
					"\xcf\xde\xd6\xc6\xb6\xf8\xb1\xbb\xca\xa1\xc2\xd4\xa1\xbf"
					"\033[m", duplicated);
		}
		fclose(fp);
	}
}

int main(int argc, char **argv)
{
	chdir(BBSHOME);

	config_t configs[] = {
		{ "day", "\xc8\xd5\xca\xae", 10, 1, true }, // 日十
		{ "week", "\xd6\xdc\xce\xe5\xca\xae", 50, 7, false }, // 周五十
		{ "month", "\xd4\xc2\xd2\xbb\xb0\xd9", 100, 30, false }, // 月一百
		{ "year", "\xc4\xea\xb6\xfe\xb0\xd9", 200, 365, false }, // 年二百
	};

	initialize_environment(INIT_DB | INIT_MDB | INIT_CONV);
	bitset_t *bs = board_init();

	for (int i = 0; i < ARRAY_SIZE(configs); ++i) {
		stat_t stat = {
			.config = configs + i, .bs = bs, .size = 0,
		};
		if (stat.config->per_board_limit)
			stat.config->limit *= BACKUP_FACTOR;

		stat.topics = malloc(stat.config->limit * sizeof(*stat.topics));
		stat.stamp = fb_time() - stat.config->days * 24 * 60 * 60;
		stat.pool = pool_create(0);
		hash_create(&stat.hash, 0, NULL);

		post_index_record_for_recent(hot_topics_stat, &stat);
		qsort(stat.topics, stat.size, sizeof(*stat.topics), topic_sort);
		print_stat(&stat);

		hash_destroy(&stat.hash);
		pool_destroy(stat.pool);
		free(stat.topics);
	}

	free(bs);
	free(bnames);
}
