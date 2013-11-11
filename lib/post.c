#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <sys/uio.h>
#include "bbs.h"
#include "mmap.h"
#include "fbbs/brc.h"
#include "fbbs/convert.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/post.h"
#include "fbbs/session.h"
#include "fbbs/string.h"

/** 文章ID序列 @mdb_string */
#define POST_ID_KEY  "post_id_seq"

static post_id_t current_post_id(void)
{
	return mdb_integer(0, "GET", POST_ID_KEY);
}

static post_id_t next_post_id(void)
{
	return mdb_integer(0, "INCR", POST_ID_KEY);
}

int post_index_cmp(const void *p1, const void *p2)
{
	const post_index_board_t *r1 = p1, *r2 = p2;
	return r1->id - r2->id;
}

int post_index_board_open_file(const char *file, record_perm_e rdonly, record_t *rec)
{
	return record_open(file, post_index_cmp, sizeof(post_index_board_t),
			rdonly, rec);
}

int post_index_board_open(int bid, record_perm_e rdonly, record_t *rec)
{
	char file[HOMELEN];
	snprintf(file, sizeof(file), "brdidx/%d", bid);
	return post_index_board_open_file(file, rdonly, rec);
}

int post_index_board_open_sticky(int bid, record_perm_e rdonly, record_t *rec)
{
	char file[HOMELEN];
	snprintf(file, sizeof(file), "brdidx/%d.sticky", bid);
	return post_index_board_open_file(file, rdonly, rec);
}

enum {
	POST_INDEX_BOARD_BUF_SIZE = 50,
	POST_INDEX_TRASH_BUF_SIZE = 50,
};

int post_index_board_to_info(post_index_record_t *pir,
		const post_index_board_t *pib, post_info_t *pi, int count)
{
	post_index_t buf;
	for (int i = 0; i < count; ++i) {
		post_index_record_read(pir, pib->id, &buf);

		pi->id = pib->id;
		pi->reid = pib->id - pib->reid_delta;
		pi->tid = pib->id - pib->tid_delta;
		pi->flag = pib->flag;
		pi->uid = pib->uid;
		pi->stamp = buf.stamp;
		pi->bid = buf.bid;
		pi->replies = buf.replies;
		pi->comments = buf.comments;
		pi->score = buf.score;
		strlcpy(pi->owner, buf.owner, sizeof(pi->owner));
		strlcpy(pi->utf8_title, buf.utf8_title, sizeof(pi->utf8_title));
		pi->estamp = 0;
		pi->ename[0] = '\0';

		++pib;
		++pi;
	}
	return count;
}

int post_index_trash_to_info(post_index_record_t *pir,
		const post_index_trash_t *pit, post_info_t *pi, int count)
{
	for (int i = 0; i < count; ++i) {
		post_info_t *pii = pi + i;
		post_index_board_to_info(pir, (post_index_board_t *) (pit + i),
				pii, 1);
		pii->estamp = (pit + i)->estamp;
		strlcpy(pii->ename, pit->ename, sizeof(pii->ename));
	}
	return count;
}

#define POST_INDEX_READ_HELPER(type, bufsize, converter)  \
	type read_buf[bufsize]; \
	while (size > 0) { \
		int max = bufsize; \
		max = size > max ? max : size; \
\
		int count = record_read(rec, read_buf, max); \
		if (count <= 0) \
			break; \
\
		converter(pir, read_buf, buf, count); \
		records += count; \
		size -= max; \
	}

int post_index_board_read(record_t *rec, int base, post_index_record_t *pir,
		post_info_t *buf, int size, post_list_type_e type)
{
	if (record_seek(rec, base, RECORD_SET) < 0)
		return 0;

	int records = 0;
	if (is_deleted(type)) {
		POST_INDEX_READ_HELPER(post_index_trash_t, POST_INDEX_TRASH_BUF_SIZE,
				post_index_trash_to_info);
	} else {
		POST_INDEX_READ_HELPER(post_index_board_t, POST_INDEX_BOARD_BUF_SIZE,
				post_index_board_to_info);
	}
	return records;
}

int post_index_trash_cmp(const void *p1, const void *p2)
{
	const post_index_trash_t *r1 = p1, *r2 = p2;
	int diff = r1->estamp - r2->estamp;
	if (diff)
		return diff;
	return r1->id - r2->id;
}

int post_index_trash_open(int bid, post_index_trash_e trash, record_t *rec)
{
	char file[HOMELEN];
	if (trash)
		snprintf(file, sizeof(file), "brdidx/%d.trash", bid);
	else
		snprintf(file, sizeof(file), "brdidx/%d.junk", bid);
	return record_open(file, post_index_trash_cmp, sizeof(post_index_trash_t),
			RECORD_WRITE, rec);
}

enum {
	POST_INDEX_PER_FILE = 100000,
};

void post_index_record_open(post_index_record_t *pir)
{
	pir->base = 0;
	pir->rdonly = RECORD_READ;
}

static post_id_t post_index_record_base(post_id_t id)
{
	return (id - 1) / POST_INDEX_PER_FILE * POST_INDEX_PER_FILE + 1;
}

static int post_index_record_cmp(const void *p1, const void *p2)
{
	const post_index_t *pi1 = p1, *pi2 = p2;
	return pi1->id - pi2->id;
}

static int post_index_record_open_file(post_id_t id, record_perm_e rdonly,
		record_t *record)
{
	char file[HOMELEN];
	snprintf(file, sizeof(file), "index/%"PRIdPID,
			(id - 1) / POST_INDEX_PER_FILE);
	return record_open(file, post_index_record_cmp, sizeof(post_index_t),
			rdonly, record);
}

static int post_index_record_check(post_index_record_t *pir, post_id_t id,
		record_perm_e rdonly)
{
	post_id_t base = post_index_record_base(id);
	if (pir->base > 0 && pir->base == base && (rdonly || !pir->rdonly))
		return 0;

	if (pir->base > 0)
		record_close(&pir->record);

	pir->base = base;
	pir->rdonly = rdonly;

	return post_index_record_open_file(id, rdonly, &pir->record);
}

int post_index_record_read(post_index_record_t *pir, post_id_t id,
		post_index_t *pi)
{
	if (post_index_record_check(pir, id, RECORD_READ) >= 0) {
		return record_read_after(&pir->record, pi, 1, id - pir->base);
	}
	memset(pi, 0, sizeof(*pi));
	return 0;
}

int post_index_record_update(post_index_record_t *pir, const post_index_t *pi)
{
	if (post_index_record_check(pir, pi->id, RECORD_WRITE) < 0)
		return 0;
	return record_write(&pir->record, pi, 1, pi->id - pir->base);
}

void post_index_record_get_title(post_index_record_t *pir, post_id_t id,
		char *buf, size_t size)
{
	if (post_index_record_check(pir, id, RECORD_READ) < 0) {
		buf[0] = '\0';
	} else {
		post_index_t pi;
		post_index_record_read(pir, id, &pi);
		strlcpy(buf, pi.utf8_title, size);
	}
}

static int post_index_record_lock(post_index_record_t *pir, record_lock_e lock,
		post_id_t id)
{
	record_perm_e perm = (lock == RECORD_WRLCK) ? RECORD_WRITE : RECORD_READ;
	if (post_index_record_check(pir, id, perm) < 0)
		return -1;
	return record_lock(&pir->record, lock, id - pir->base, RECORD_SET, 1);
}

static record_callback_e post_index_record_for_file(post_id_t base,
		post_id_t end, post_index_record_callback_t callback, void *args)
{
	record_t record;
	if (post_index_record_open_file(base, RECORD_READ, &record) < 0)
		return RECORD_CALLBACK_BREAK;

	if (!end)
		end = base + POST_INDEX_PER_FILE;
	int offset = end - base;

	record_callback_e ret = RECORD_CALLBACK_CONTINUE;
	do {
		post_index_t buf[64];
		offset -= ARRAY_SIZE(buf);
		if (offset < 0)
			offset = 0;

		int count = record_read_after(&record, buf, ARRAY_SIZE(buf), offset);
		if (count <= 0) {
			ret = RECORD_CALLBACK_BREAK;
			break;
		}

		for (int i = count - 1; i >= 0; --i) {
			if (callback(buf + i, args) == RECORD_CALLBACK_BREAK) {
				ret = RECORD_CALLBACK_BREAK;
				break;
			}
		}
	} while (ret != RECORD_CALLBACK_BREAK && offset > 0);

	record_close(&record);
	return ret;
}

int post_index_record_for_recent(post_index_record_callback_t cb, void *args)
{
	post_id_t id = current_post_id();
	if (id) {
		post_id_t base = post_index_record_base(id);
		post_id_t end = id + 1;
		while (base >= 0 && post_index_record_for_file(base, end, cb, args)
				!= RECORD_CALLBACK_BREAK) {
			end = 0;
			base -= POST_INDEX_PER_FILE;
		}
	}
	return 0;
}

void post_index_record_close(post_index_record_t *pir)
{
	if (pir->base >= 0)
		record_close(&pir->record);
	pir->base = 0;
	pir->rdonly = RECORD_READ;
}

enum {
	POST_CONTENT_PER_FILE = 10000,
};

typedef struct {
	uint32_t offset;
	uint32_t length;
} post_content_header_t;

static char *post_content_file_name(post_id_t id, char *file, size_t size)
{
	snprintf(file, size, "post/%"PRIdPID, (id - 1) / POST_CONTENT_PER_FILE);
	return file;
}

enum {
	POST_CONTENT_NOT_EXIST = 0,
	POST_CONTENT_NEED_LOCK = 1,
	POST_CONTENT_READ_ERROR = 2,
};

static char *post_content_try_read(int fd, post_id_t id, char *buf,
		size_t *size)
{
	int relative_id = (id - 1) % POST_CONTENT_PER_FILE;

	post_content_header_t header;
	lseek(fd, relative_id * sizeof(header), SEEK_SET);
	file_read(fd, &header, sizeof(header));

	if (!header.offset) {
		buf[0] = POST_CONTENT_NOT_EXIST;
		return NULL;
	}

	if (!header.length) {
		buf[0] = '\0';
		return buf;
	}

	char *sbuf = buf;
	if (header.length >= *size) {
		*size = header.length + 1;
		sbuf = malloc(*size);
	}

	char hdr[3];
	struct iovec vec[] = {
		{ .iov_base = hdr, .iov_len = sizeof(hdr) },
		{ .iov_base = sbuf, .iov_len = header.length + 1 },
	};
	lseek(fd, header.offset, SEEK_SET);
	int ret = readv(fd, vec, ARRAY_SIZE(vec));

	if (ret != sizeof(hdr) + header.length + 1) {
		if (sbuf != buf)
			free(sbuf);
		buf[0] = POST_CONTENT_READ_ERROR;
		return NULL;
	}

	uint16_t rel = *(uint16_t *) (hdr + 1);
	if (hdr[0] != '\n' || rel != relative_id || sbuf[header.length] != '\0') {
		if (sbuf != buf)
			free(sbuf);
		buf[0] = POST_CONTENT_NEED_LOCK;
		return NULL;
	}
	return sbuf;
}

char *post_content_read_fd(int fd, post_id_t id, char *buf, size_t *size)
{
	char *ptr = post_content_try_read(fd, id, buf, size);
	if (!ptr && buf[0] == POST_CONTENT_NEED_LOCK) {
		file_lock(fd, FILE_WRLCK, 0, FILE_SET, 0);
		ptr = post_content_try_read(fd, id, buf, size);
		file_lock(fd, FILE_UNLCK, 0, FILE_SET, 0);
	}
	return ptr;
}

char *post_content_read(post_id_t id, char *buf, size_t size)
{
	char file[HOMELEN];
	post_content_file_name(id, file, sizeof(file));

	int fd = open(file, O_RDONLY);
	if (fd < 0)
		return NULL;

	char *ptr = post_content_read_fd(fd, id, buf, &size);

	close(fd);
	return ptr;
}

int post_content_write(post_id_t id, const char *str, size_t size)
{
	char file[HOMELEN];
	post_content_file_name(id, file, sizeof(file));

	int ret = -1;
	int fd = open(file, O_WRONLY | O_CREAT);
	if (fd < 0)
		return -1;

	if (file_lock_all(fd, FILE_WRLCK) < 0)
		goto e1;
	struct stat st;
	if (fstat(fd, &st) < 0)
		goto e2;

	uint32_t offset = st.st_size;
	if (offset < sizeof(uint32_t) * POST_CONTENT_PER_FILE)
		offset = sizeof(uint32_t) * POST_CONTENT_PER_FILE;

	post_id_t base = 1 + (id - 1) / POST_CONTENT_PER_FILE
			* POST_CONTENT_PER_FILE;
	uint16_t rel = id - base;
	lseek(fd, rel * sizeof(post_content_header_t), SEEK_SET);

	char buf[3] = { '\n' };
	memcpy(buf + 1, &rel, sizeof(rel));
	if (UINT32_MAX - offset < sizeof(buf) + size + 1)
		goto e2;

	post_content_header_t header = { .offset = offset, .length = size };
	file_write(fd, &header, sizeof(header));

	struct iovec vec[] = {
		{ .iov_base = buf, .iov_len = sizeof(buf) },
		{ .iov_base = (void *) str, .iov_len = size + 1 },
	};
	lseek(fd, offset, SEEK_SET);
	ret = writev(fd, vec, ARRAY_SIZE(vec));

e2: file_lock_all(fd, FILE_UNLCK);
e1: close(fd);
	return ret;
}

void post_content_record_open(post_content_record_t *pcr)
{
	pcr->base = 0;
	pcr->fd = -1;
	pcr->result = pcr->buf;
	pcr->size = sizeof(pcr->buf);
}

static post_id_t post_content_record_base(post_id_t id)
{
	return (id - 1) / POST_CONTENT_PER_FILE * POST_CONTENT_PER_FILE + 1;
}

char *post_content_record_read(post_content_record_t *pcr, post_id_t id)
{
	post_id_t base = post_content_record_base(id);
	if (pcr->base != base) {
		if (pcr->fd >= 0)
			file_close(pcr->fd);

		char file[HOMELEN];
		post_content_file_name(id, file, sizeof(file));

		pcr->fd = open(file, O_RDONLY);
		pcr->base = base;
	}

	if (!pcr->result) {
		pcr->result = pcr->buf;
		pcr->size = sizeof(pcr->buf);
	}

	char *ptr = NULL;
	if (pcr->fd >= 0)
		ptr = post_content_read_fd(pcr->fd, id, pcr->result, &pcr->size);

	if (ptr != pcr->result && pcr->result != pcr->buf)
		free(pcr->result);
	pcr->result = ptr;
	return ptr;
}

void post_content_record_close(post_content_record_t *pcr)
{
	if (pcr->fd >= 0)
		file_close(pcr->fd);
	if (pcr->result != pcr->buf)
		free(pcr->result);
}

static record_callback_e post_sticky_filter(void *ptr, void *fargs, int offset)
{
	const post_index_board_t *pib = ptr;
	post_id_t *id = fargs;
	return pib->id == *id ? RECORD_CALLBACK_MATCH : RECORD_CALLBACK_CONTINUE;
}

int post_remove_sticky(int bid, post_id_t id)
{
	record_t record;
	if (post_index_board_open_sticky(bid, RECORD_WRITE, &record) < 0)
		return 0;
	int r = record_delete(&record, NULL, 0, post_sticky_filter, &id);
	record_close(&record);
	return r;
}

int post_add_sticky(int bid, const post_info_t *pi)
{
	record_t record;
	if (post_index_board_open_sticky(bid, RECORD_WRITE, &record) < 0)
		return 0;

	post_index_board_t pib = {
		.id = pi->id,
		.reid_delta = pi->id - pi->reid,
		.tid_delta = pi->id - pi->tid,
		.uid = pi->uid,
		.flag = pi->flag | POST_FLAG_STICKY,
	};

	record_lock_all(&record, RECORD_WRLCK);
	int r = 0;
	int count = record_count(&record);
	if (count < MAX_NOTICE) {
		if (record_search(&record, post_sticky_filter, &pib.id, -1, false) < 0)
			r = record_append(&record, &pib, 1);
	}
	record_lock_all(&record, RECORD_UNLCK);

	record_close(&record);
	return r;
}

bool reorder_sticky_posts(int bid, post_id_t pid)
{
	bool success = false;
	record_t record;
	if (post_index_board_open_sticky(bid, RECORD_WRITE, &record) < 0)
		goto e1;
	if (record_lock_all(&record, RECORD_WRLCK) < 0)
		goto e2;

	post_index_board_t pibs[MAX_NOTICE];
	int count = record_read(&record, pibs, ARRAY_SIZE(pibs));
	if (count > 1) {
		for (int i = 0; i < count - 1; ++i) {
			if (pibs[i].id == pid) {
				post_index_board_t temp = pibs[i];
				memmove(pibs + i, pibs + i + 1,
						(count - 1 - i) * sizeof(pibs[0]));
				pibs[count - 1] = temp;
				break;
			}
		}
		if (record_write(&record, pibs, count, 0) > 0)
			success = true;
	}

	record_lock_all(&record, RECORD_UNLCK);
e2:	record_close(&record);
e1: return success;
}

char *convert_file_to_utf8_content(const char *file)
{
	char *utf8_content = NULL;
	mmap_t m = { .oflag = O_RDONLY };
	if (mmap_open(file, &m) == 0) {
		utf8_content = malloc(m.size * 2);
		convert(env_g2u, m.ptr, CONVERT_ALL, utf8_content, m.size * 2,
				NULL, NULL);
		mmap_close(&m);
	}
	return utf8_content;
}

enum {
	SIGNATURE_LINE_LEN = 256,
};

static void read_signature_legacy(const char *uname, int offset, char *buffer,
		size_t size)
{
	char *out = buffer;
	size_t remain = size;
	strappend(&out, &remain, "\n--\n");
	if (offset <= 0)
		return;

	char file[HOMELEN];
	sethomefile(file, uname, "signatures");
	FILE *fp = fopen(file, "r");
	if (!fp)
		return;

	char buf[256];
	for (int i = (offset - 1) * MAXSIGLINES; i > 0; --i) {
		if (!fgets(buf, sizeof(buf), fp)) {
			fclose(fp);
			return;
		}
	}

	int blank = 0;
	for (int i = 0; i < MAXSIGLINES; ++i) {
		if (!fgets(buf, sizeof(buf), fp))
			break;
		if (buf[0] == '\n' || streq(buf, "\r\n")) {
			++blank;
		} else {
			while (blank-- > 0)
				strappend(&out, &remain, "\n");
			blank = 0;
			//% ":·" "·[FROM:"
			if (!strstr(buf, ":\xa1\xa4"BBSNAME" "BBSHOST"\xa1\xa4[FROM:"))
				strappend(&out, &remain, buf);
		}
	}
	fclose(fp);
}

static char *generate_content(const post_request_t *pr, const char *uname,
		const char *nick, const char *ip, bool anony, size_t length)
{
	UTF8_BUFFER(nick, NAMELEN);
	convert_g2u(nick, utf8_nick);

	char header[512];
	snprintf(header, sizeof(header),
			"发信人: %s (%s), 信区: %s\n标  题: %s\n发信站: %s (%s)\n\n",
			uname, utf8_nick, pr->board->name, pr->title, BBSNAME_UTF8,
			format_time(fb_time(), TIME_FORMAT_UTF8_ZH));
	int header_len = strlen(header);

	int content_len = length ? length : strlen(pr->content);
	if (pr->cp)
		content_len *= 2;

	char signature[MAXSIGLINES * SIGNATURE_LINE_LEN + 5];
	char utf8_signature[sizeof(signature) * 2 + 1];
	if (!anony && pr->sig > 0) {
		read_signature_legacy(uname, pr->sig, signature, sizeof(signature));
		convert_g2u(signature, utf8_signature);
	} else {
		strlcpy(utf8_signature, "\n--", sizeof(utf8_signature));
	}
	int signature_len = strlen(utf8_signature);

	char source[256] = { '\0' };
	if (ip) {
		char utf8_ip[80];
		convert_g2u(ip, utf8_ip);
		snprintf(source, sizeof(source), "\033[m\033[1;%2dm※ %s:·"BBSNAME_UTF8
				" "BBSHOST"·%s[FROM: %s]\033[m\n", 31 + rand() % 7,
				pr->crosspost ? "转载" : "来源", pr->web ? "HTTP " : "",
				utf8_ip);
	}
	int source_len = strlen(source);

	int total_len = header_len + content_len + signature_len + source_len + 2;
	char *content = malloc(total_len);

	memcpy(content, header, header_len);
	if (pr->cp)
		convert(pr->cp, pr->content, CONVERT_ALL, content + header_len,
				total_len - header_len, NULL, NULL);
	else
		strlcpy(content + header_len, pr->content, total_len - header_len);

	int len = strlen(content);
	if (len < total_len)
		memcpy(content + len, utf8_signature, total_len - len);
	len += signature_len;
	if (content[len - 1] != '\n' && len < total_len) {
		content[len++] = '\n';
	}
	if (len < total_len)
		memcpy(content + len, source, total_len - len);
	content[len + source_len] = '\0';
	return content;
}

#define BOARD_POST_COUNT_KEY "board_post_count"

int get_board_post_count(int bid)
{
	return mdb_integer(0, "HGET", BOARD_POST_COUNT_KEY " %d", bid);
}

static void set_board_post_count(int bid, int count)
{
	mdb_integer(0, "HSET", BOARD_POST_COUNT_KEY " %d %d", bid, count);
}

static post_id_t insert_post(const post_request_t *pr, const char *uname,
		const char *content, fb_time_t now)
{
	post_index_t pi = { .id = next_post_id(), };
	if (pi.id) {
		pi.reid_delta = pr->reid ? pi.id - pr->reid : 0;
		pi.tid_delta = pr->tid ? pi.id - pr->tid : 0;
		pi.stamp = now;
		pi.uid = get_user_id(uname);
		pi.flag = (pr->marked ? POST_FLAG_MARKED : 0)
				| (pr->locked ? POST_FLAG_LOCKED : 0);
		pi.bid = pr->board->id;
		strlcpy(pi.owner, uname, sizeof(pi.owner));
		strlcpy(pi.utf8_title, pr->title, sizeof(pi.utf8_title));

		post_content_write(pi.id, content, strlen(content));

		post_index_record_t pir;
		post_index_record_open(&pir);
		if (!post_index_record_update(&pir, &pi))
			pi.id = 0;
		post_index_record_close(&pir);
	}
	if (pi.id) {
		post_index_board_t pib = {
			.id = pi.id, .reid_delta = pi.reid_delta, .tid_delta = pi.tid_delta,
			.uid = pi.uid, .flag = pi.flag, .stamp = now, .cstamp = 0,
		};

		record_t record;
		post_index_board_open(pi.bid, RECORD_WRITE, &record);
		record_lock_all(&record, RECORD_WRLCK);
		if (record_append(&record, &pib, 1) < 0)
			pi.id = 0;
		record_lock_all(&record, RECORD_UNLCK);
		set_board_post_count(pi.bid, record_count(&record));
		record_close(&record);
	}

	return pi.id;
}

/**
 * Publish a post.
 * @param pr The post request.
 * @return file id on success, -1 on error.
 */
post_id_t publish_post(const post_request_t *pr)
{
	if (!pr || !pr->title || (!pr->content && !pr->gbk_file) || !pr->board)
		return 0;

	bool anony = pr->anony && (pr->board->flag & BOARD_ANONY_FLAG);
	const char *uname = NULL, *nick = NULL, *ip = pr->ip;
	if (anony) {
		uname = ANONYMOUS_ACCOUNT;
		nick = ANONYMOUS_NICK;
		ip = ANONYMOUS_SOURCE;
	} else if (pr->user) {
		uname = pr->user->userid;
		nick = pr->user->username;
	} else if (pr->autopost) {
		uname = pr->uname;
		nick = pr->nick;
	}
	if (!uname || !nick)
		return 0;

	char *content;
	if (pr->gbk_file)
		content = convert_file_to_utf8_content(pr->gbk_file);
	else
		content = generate_content(pr, uname, nick, ip, anony, pr->length);

	fb_time_t now = fb_time();
	post_id_t pid = insert_post(pr, uname, content, now);
	free(content);

	if (pid) {
		set_last_post_time(pr->board->id, now);

		if (!pr->autopost) {
			brc_initialize(uname, pr->board->name);
			brc_mark_as_read(now);
			brc_update(uname, pr->board->name);
		}
	}
	return pid;
}

enum {
	MAX_QUOTED_LINES = 5,     ///< Maximum quoted lines (for QUOTE_AUTO).
	/** A line will be truncated at this width (78 for quoted line) */
	TRUNCATE_WIDTH = 76,
};

/**
 * Find newline in [begin, end), truncate at TRUNCATE_WIDTH.
 * @param begin The head pointer.
 * @param end The off-the-end pointer.
 * @return Off-the-end pointer to the first (truncated) line.
 */
static const char *get_truncated_line(const char *begin, const char *end,
		bool utf8)
{
	const char *code = "[0123456789;";
	bool ansi = false;

	int width = TRUNCATE_WIDTH;
	if (end - begin >= 2 && *begin == ':' && *(begin + 1) == ' ')
		width += 2;

	for (const char *s = begin; s < end; ++s) {
		if (*s == '\n')
			return s + 1;

		if (*s == '\033') {
			ansi = true;
			continue;
		}

		if (ansi) {
			if (!memchr(code, *s, sizeof(code) - 1))
				ansi = false;
			continue;
		}

		if (*s & 0x80) {
			width -= 2;
			if (width < 0)
				return s;
			++s;
			if (width == 0)
				return (s + 1 > end ? end : s + 1);
		} else {
			if (--width == 0)
				return s + 1;
		}
	}
	return end;
}

/**
 * Tell if a line is meaningless.
 * @param begin The beginning of the line.
 * @param end The off-the-end pointer.
 * @return True if str is quotation of a quotation or contains only white
           spaces, false otherwise.
 */
static bool qualify_quotation(const char *begin, const char *end)
{
	const char *s = begin;
	if (end - s > 2 && (*s == ':' || *s == '>') && *(s + 1) == ' ') {
		s += 2;
		if (end - s > 2 && (*s == ':' || *s == '>') && *(s + 1) == ' ')
			return false;
	}

	while (s < end && (*s == ' ' || *s == '\t' || *s == '\r'))
		++s;

	return (s < end && *s != '\n');
}

typedef size_t (*filter_t)(const char *, size_t, FILE *);

static size_t default_filter(const char *s, size_t size, FILE *fp)
{
	return fwrite(s, size, 1, fp);
}

static const char *get_newline(const char *begin, const char *end)
{
	while (begin < end) {
		if (*begin++ == '\n')
			return begin;
	}
	return begin;
}

#define PRINT_CONST_STRING(s)  (*filter)(s, sizeof(s) - 1, fp)

static void quote_author(const char *begin, const char *lend, bool mail,
		bool utf8, FILE *fp, filter_t filter)
{
	const char *quser = begin, *ptr = lend;
	while (quser < lend) {
		if (*quser++ == ' ')
			break;
	}
	while (--ptr >= begin) {
		if (*ptr == ')')
			break;
	}
	++ptr;

	if (utf8)
		PRINT_CONST_STRING("\n【 在 ");
	else
		PRINT_CONST_STRING("\n\xa1\xbe \xd4\xda ");
	if (ptr > quser)
		filter(quser, ptr - quser, fp);
	if (utf8)
		PRINT_CONST_STRING(" 的");
	else
		PRINT_CONST_STRING(" \xb5\xc4");
	if (mail) {
		if (utf8)
			PRINT_CONST_STRING("来信");
		else
			PRINT_CONST_STRING("\xc0\xb4\xd0\xc5");
	} else {
		if (utf8)
			PRINT_CONST_STRING("大作");
		else
			PRINT_CONST_STRING("\xb4\xf3\xd7\xf7");
	}
	if (utf8)
		PRINT_CONST_STRING("中提到: 】\n");
	else
		PRINT_CONST_STRING("\xd6\xd0\xcc\xe1\xb5\xbd: \xa1\xbf\n");
}

static const char *find_char(const char *begin, const char *end, int c)
{
	for (const char *p = begin; p < end ; ++p) {
		if (*p == c)
			return p;
	}
	return NULL;
}

static const char *reverse_find_char(const char *begin, const char *end, int c)
{
	for (const char *p = end - 1; p >= begin; --p) {
		if (*p == c)
			return p;
	}
	return NULL;
}

static void quote_author_pack(const char *begin, const char *end, FILE *fp,
		filter_t filter)
{
	const char *user_begin = begin + sizeof("发信人: ") - 1;
	const char *lend = get_newline(begin, end);
	const char *user_end = reverse_find_char(begin, lend, ',');

	begin = get_newline(lend, end);
	lend = get_newline(begin, end);

	const char *date_begin = find_char(begin, lend, '(');
	if (date_begin)
		++date_begin;
	const char *date_end = find_char(begin, lend, ')');

	PRINT_CONST_STRING("    \033[0;1;32m");
	if (user_end && user_end > user_begin)
		filter(user_begin, user_end - user_begin, fp);
	PRINT_CONST_STRING(" 于 \033[1;36m");
	if (date_begin && date_end > date_begin)
		filter(date_begin, date_end - date_begin, fp);
	PRINT_CONST_STRING("\033[0;1m 提到：\033[0m\n\n");
}

#define GBK_SOURCE  "\xa1\xf9 \xc0\xb4\xd4\xb4:\xa1\xa4"
#define UTF8_SOURCE  "※ 来源:·"

/**
 * Make quotation from a string.
 * @param str String to be quoted.
 * @param size Size of the string.
 * @param fp Output stream. If NULL, will output to stdout (web).
 * @param mode Quotation mode. See QUOTE_* enums.
 * @param mail Whether the referenced post is a mail.
 * @param filter Output filter function.
 */
void quote_string(const char *str, size_t size, FILE *fp,
		post_quote_e mode, bool mail, bool utf8, filter_t filter)
{
	if (!filter)
		filter = default_filter;

	const char *begin = str, *end = str + size;
	const char *lend = get_newline(begin, end);
	if (mode == QUOTE_PACK || mode == QUOTE_PACK_COMPACT)
		quote_author_pack(begin, end, fp, filter);
	else
		quote_author(begin, lend, mail, utf8, fp, filter);

	bool header = true, tail = false;
	size_t lines = 0;
	const char *ptr;
	while (1) {
		ptr = lend;
		if (ptr >= end)
			break;

		if (mode == QUOTE_PACK || mode == QUOTE_PACK_COMPACT)
			lend = get_newline(ptr, end);
		else
			lend = get_truncated_line(ptr, end, utf8);
		if (header && *ptr == '\n') {
			header = false;
			continue;
		}

		if (lend - ptr == 3 && !memcmp(ptr, "--\n", 3)) {
			tail = true;
			if (mode == QUOTE_LONG || mode == QUOTE_AUTO
					|| mode == QUOTE_PACK || mode == QUOTE_PACK_COMPACT)
				break;
		}

		if (!header || mode == QUOTE_ALL) {
			if ((mode == QUOTE_LONG || mode == QUOTE_AUTO)
					&& !qualify_quotation(ptr, lend)) {
				if (*(lend - 1) != '\n')
					lend = get_newline(lend, end);
				continue;
			}

			if (mode == QUOTE_SOURCE && !utf8
					&& lend - ptr > 10 + sizeof(GBK_SOURCE)
					&& !memcmp(ptr + 10, GBK_SOURCE, sizeof(GBK_SOURCE))) {
				break;
			}

			if (mode == QUOTE_SOURCE && utf8
					&& lend - ptr > 10 + sizeof(UTF8_SOURCE)
					&& !memcmp(ptr + 10, UTF8_SOURCE, sizeof(UTF8_SOURCE))) {
				break;
			}

			if (mode == QUOTE_AUTO) {
				if (++lines > MAX_QUOTED_LINES) {
					if (utf8) {
						PRINT_CONST_STRING(": .................（以下省略）");
					} else {
						PRINT_CONST_STRING(": .................\xa3\xa8"
								"\xd2\xd4\xcf\xc2\xca\xa1\xc2\xd4\xa3\xa9");
					}
					break;
				}
			}

			if (mode != QUOTE_SOURCE && mode != QUOTE_PACK
					&& mode != QUOTE_PACK_COMPACT)
				PRINT_CONST_STRING(": ");
			(*filter)(ptr, lend - ptr, fp);
			if (*(lend - 1) != '\n')
				PRINT_CONST_STRING("\n");
		}
	}
}

void quote_file_(const char *orig, const char *output, post_quote_e mode,
		bool mail, bool utf8, filter_t filter)
{
	if (mode != QUOTE_NOTHING) {
		FILE *fp = fopen(output, "w");
		if (fp) {
			mmap_t m = { .oflag = O_RDONLY };
			if (mmap_open(orig, &m) == 0) {
				quote_string(m.ptr, m.size, fp, mode, mail, utf8, filter);
				mmap_close(&m);
			}
			fclose(fp);
		}
	}
}

typedef struct {
	post_index_record_t *pir;
	post_filter_t *filter;
	post_id_t id;
	bool set;
	bool toggle;
	post_flag_e flag;
} post_index_board_update_flag_t;

bool match_filter(const post_index_board_t *pib,
		post_index_record_t *pir, const post_filter_t *filter, int offset)
{
	bool match = true;
	if (filter->uid)
		match &= pib->uid == filter->uid;
	if (filter->min)
		match &= pib->id >= filter->min;
	if (filter->max)
		match &= pib->id <= filter->max;
	if (filter->tid)
		match &= pib->id - pib->tid_delta == filter->tid;
	if (filter->flag)
		match &= (pib->flag & filter->flag) == filter->flag;
	if (filter->fake_id_min)
		match &= offset >= filter->fake_id_min - 1;
	if (filter->fake_id_max)
		match &= offset < filter->fake_id_max;
	if (filter->type == POST_LIST_TOPIC)
		match &= !pib->tid_delta;
	if (*filter->utf8_keyword) {
		UTF8_BUFFER(title, POST_TITLE_CCHARS);
		post_index_record_get_title(pir, pib->id, utf8_title,
				sizeof(utf8_title));
		match &= (bool) strcasestr(utf8_title, filter->utf8_keyword);
	}
	return match;
}

static record_callback_e post_index_board_update_flag(void *ptr, void *args,
		int offset)
{
	post_index_board_t *pib = ptr;
	post_index_board_update_flag_t *pibuf = args;

	if (pibuf->id == pib->id || (pibuf->pir && pibuf->filter
				&& match_filter(pib, pibuf->pir, pibuf->filter, offset))) {
		if (pibuf->toggle)
			pibuf->set = !(pib->flag & pibuf->flag);
		if (pibuf->set)
			pib->flag |= pibuf->flag;
		else
			pib->flag &= ~pibuf->flag;
		return RECORD_CALLBACK_MATCH;
	}
	if (!pibuf->id)
		return RECORD_CALLBACK_CONTINUE;
	COMPARE_RETURN(pib->id, pibuf->id);
}

int set_post_flag(record_t *rec, post_index_record_t *pir,
		post_filter_t *filter, post_flag_e flag, bool set, bool toggle)
{
	post_index_board_update_flag_t pibuf = {
		.pir = pir, .filter = filter, .id = 0,
		.set = set, .toggle = toggle, .flag = flag,
	};
	return record_update(rec, NULL, 0, post_index_board_update_flag, &pibuf);
}

int set_post_flag_one(record_t *rec, post_index_board_t *pib, int offset,
		post_flag_e flag, bool set, bool toggle)
{
	post_index_board_update_flag_t pibuf = {
		.pir = NULL, .filter = NULL, .id = pib->id,
		.set = set, .toggle = toggle, .flag = flag,
	};
	return record_update(rec, pib, offset,
			post_index_board_update_flag, &pibuf);
}

bool is_deleted(post_list_type_e type)
{
	return type == POST_LIST_TRASH || type == POST_LIST_JUNK;
}

int dump_content_to_gbk_file(const char *utf8_str, size_t length, char *file,
		size_t size)
{
	snprintf(file, size, "tmp/gbk_dump.%d", getpid());
	FILE *fp = fopen(file, "w");
	if (!fp)
		return -1;
	convert_to_file(env_u2g, utf8_str, length, fp);
	fclose(fp);
	return 0;
}

/** 版面最新一篇文章的时间 @mdb_hash */
#define LAST_POST_KEY  "last_post"

bool set_last_post_time(int bid, fb_time_t stamp)
{
	return mdb_cmd("HSET", LAST_POST_KEY " %d %"PRIdFBT, bid, stamp);
}

fb_time_t get_last_post_time(int bid)
{
	return (fb_time_t) mdb_integer(0, "HGET", LAST_POST_KEY " %d", bid);
}

static void adjust_user_post_count(const char *uname, int delta)
{
	struct userec urec;
	int unum = searchuser(uname);
	getuserbyuid(&urec, unum);
	urec.numposts += delta;
	substitut_record(NULL, &urec, sizeof(urec), unum);
}

typedef struct {
	post_index_record_t *pir;
	const post_filter_t *filter;
	bool delete_;
} post_deletion_callback_t;

static record_callback_e post_deletion_callback(void *ptr, void *args,
		int offset)
{
	post_index_trash_t *pit = ptr;
	post_deletion_callback_t *pdc = args;
	post_index_t pi;

	if (match_filter((post_index_board_t *) pit, pdc->pir,
				pdc->filter, offset)) {
		if (post_index_record_lock(pdc->pir, RECORD_WRLCK, pit->id) == 0) {
			post_index_record_read(pdc->pir, pit->id, &pi);
			if (pdc->delete_)
				pi.flag |= POST_FLAG_DELETED;
			else
				pi.flag &= ~POST_FLAG_DELETED;
			post_index_record_update(pdc->pir, &pi);
			post_index_record_lock(pdc->pir, RECORD_UNLCK, pit->id);

			if (pit->flag & POST_FLAG_JUNK)
				adjust_user_post_count(pi.owner, pdc->delete_ ? -1 : 1);
		}
		return RECORD_CALLBACK_MATCH;
	}
	return RECORD_CALLBACK_CONTINUE;
}

static int post_deletion_trigger(record_t *trash, const post_filter_t *filter,
		post_index_record_t *pir, bool deletion)
{
	post_deletion_callback_t pdc = {
		.pir = pir, .filter = filter, .delete_ = deletion,
	};
	int rows = record_update(trash, NULL, 0, post_deletion_callback, &pdc);
	return rows;
}

typedef struct {
	post_index_record_t *pir;
	const post_filter_t *filter;
	record_t *trash;
	const char *ename;
	fb_time_t estamp;
	bool junk;
	bool decrease;
	bool force;
} post_index_trash_insert_t;

static record_callback_e post_index_trash_insert(void *rec, void *args,
		int offset)
{
	const post_index_board_t *pib = rec;
	post_index_trash_insert_t *piti = args;

	if (match_filter(pib, piti->pir, piti->filter, offset)
			&& (piti->force || !(pib->flag & POST_FLAG_MARKED))) {
		post_index_trash_t pit = {
			.id = pib->id,
			.reid_delta = pib->reid_delta,
			.tid_delta = pib->tid_delta,
			.uid = pib->uid,
			.flag = pib->flag,
			.stamp = pib->stamp,
			.cstamp = pib->cstamp,
			.estamp = piti->estamp,
		};
		if (piti->decrease && (piti->junk || (pib->flag & POST_FLAG_WATER)))
			pit.flag |= POST_FLAG_JUNK;
		else
			pit.flag &= POST_FLAG_JUNK;
		strlcpy(pit.ename, piti->ename, sizeof(pit.ename));
		record_append(piti->trash, &pit, 1);
		return RECORD_CALLBACK_MATCH;
	}
	return RECORD_CALLBACK_CONTINUE;
}

/**
 * 删除版面上符合条件的文章
 * @param filter 文章过滤条件，其中必须指定版面 ID
 * @param ptr 该篇文章的版面记录(PIB)，供参考，可为 NULL
 * @param offset 文章在版面上的位置，供参考，可为 0
 * @param junk 是否减少作者文章数
 * @param bm_visible 是则进入版面垃圾箱，否则进入站务垃圾箱
 * @param force 是否删除带保留标记的文章
 * @return 被删除的文章数
 */
int post_index_board_delete(const post_filter_t *filter, void *ptr, int offset,
		bool junk, bool bm_visible, bool force)
{
	if (!filter->bid)
		return 0;

	bool decrease = true;
	board_t board;
	if (get_board_by_bid(filter->bid, &board) && is_junk_board(&board)) {
		decrease = false;
	}
	int deleted = 0;

	record_t record, trash;
	if (post_index_board_open(filter->bid, RECORD_WRITE, &record) < 0)
		goto e1;
	if (post_index_trash_open(filter->bid,
			bm_visible ? POST_INDEX_TRASH : POST_INDEX_JUNK, &trash) < 0)
		goto e2;
	post_index_record_t pir;
	post_index_record_open(&pir);

	post_index_trash_insert_t piti = {
		.filter = filter, .pir = &pir,
		.trash = &trash, .ename = currentuser.userid,
		.estamp = fb_time(), .junk = junk, .decrease = decrease,
	};

	if (record_lock_all(&trash, RECORD_WRLCK) < 0)
		goto e3;
	int current = record_seek(&trash, 0, RECORD_END);

	if (record_lock_all(&record, RECORD_WRLCK) < 0)
		goto e4;
	deleted = record_delete(&record, ptr, offset,
			post_index_trash_insert, &piti);
	record_lock_all(&record, RECORD_UNLCK);

	if (deleted) {
		set_board_post_count(filter->bid, record_count(&record));
		post_filter_t filter2 = { .fake_id_min = current + 1 };
		post_deletion_trigger(&trash, &filter2, &pir, true);
	}

e4: record_lock_all(&trash, RECORD_UNLCK);
e3: post_index_record_close(&pir);
e2: record_close(&trash);
e1: record_close(&record);
	return deleted;
}

typedef struct {
	post_index_record_t *pir;
	const post_filter_t *filter;
	post_index_board_t *buf;
	int count;
	int max;
} post_undeletion_callback_t;

static record_callback_e post_undeletion_callback(void *ptr, void *args,
		int offset)
{
	const post_index_trash_t *pit = ptr;
	post_undeletion_callback_t *puc = args;

	if (match_filter((post_index_board_t *) pit, puc->pir,
				puc->filter, offset)) {
		if (puc->count >= puc->max)
			return RECORD_CALLBACK_CONTINUE;

		post_index_board_t *pib = puc->buf + puc->count;
		pib->id = pit->id;
		pib->reid_delta = pit->reid_delta;
		pib->tid_delta = pit->tid_delta;
		pib->uid = pit->uid;
		pib->flag = pit->flag & ~POST_FLAG_JUNK;
		pib->stamp = pit->stamp;
		pib->cstamp = pit->cstamp;
		++puc->count;
		return RECORD_CALLBACK_MATCH;
	}
	return RECORD_CALLBACK_CONTINUE;
}

int post_index_board_undelete(const post_filter_t *filter, void *ptr,
		int offset, bool bm_visible)
{
	if (!filter->bid)
		return 0;

	record_t record, trash;
	post_index_board_open(filter->bid, RECORD_WRITE, &record);
	post_index_trash_open(filter->bid,
			bm_visible ? POST_INDEX_TRASH : POST_INDEX_JUNK, &trash);
	post_index_record_t pir;
	post_index_record_open(&pir);

	record_lock_all(&trash, RECORD_WRLCK);
	int undeleted = post_deletion_trigger(&trash, filter, &pir, false);
	post_index_board_t *buf = malloc(undeleted * sizeof(post_index_board_t));
	post_undeletion_callback_t puc = {
		.pir = &pir, .filter = filter,
		.buf = buf, .count = 0, .max = undeleted,
	};

	record_lock_all(&record, RECORD_WRLCK);
	record_delete(&trash, NULL, 0, post_undeletion_callback, &puc);
	record_merge(&record, puc.buf, puc.count);
	record_lock_all(&record, RECORD_UNLCK);
	set_board_post_count(filter->bid, record_count(&record));

	record_lock_all(&trash, RECORD_UNLCK);
	free(buf);
	post_index_record_close(&pir);
	record_close(&trash);
	record_close(&record);
	return undeleted;
}

static char *replace_content_title(const char *content, size_t len,
		const char *title)
{
	const char *end = content + len;
	const char *l1_end = get_line_end(content, end);
	const char *l2_end = get_line_end(l1_end, end);

	// sizeof("标  题: ") in UTF-8 is 10
	const char *begin = l1_end + 10;
	int orig_title_len = l2_end - begin - 1; // exclude '\n'
	if (orig_title_len < 0)
		return NULL;

	int new_title_len = strlen(title);
	len += new_title_len - orig_title_len;
	char *s = malloc(len + 1);
	char *p = s;
	size_t l = begin - content;
	memcpy(p, content, l);
	p += l;
	memcpy(p, title, new_title_len);
	p += new_title_len;
	*p++ = '\n';
	l = end - l2_end;
	memcpy(p, l2_end, end - l2_end);
	s[len] = '\0';
	return s;
}

bool alter_title(post_index_record_t *pir, const post_info_t *pi)
{
	if (post_index_record_check(pir, pi->id, RECORD_WRITE) < 0)
		return false;

	post_index_t tmp;
	post_index_record_lock(pir, RECORD_WRLCK, pi->id);
	post_index_record_read(pir, pi->id, &tmp);
	strlcpy(tmp.utf8_title, pi->utf8_title, sizeof(tmp.utf8_title));
	post_index_record_update(pir, &tmp);
	post_index_record_lock(pir, RECORD_UNLCK, pi->id);

	char buf[POST_CONTENT_BUFLEN];
	char *content = post_content_read(pi->id, buf, sizeof(buf));
	if (!content)
		return false;
	char *new_content =
		replace_content_title(content, strlen(content), pi->utf8_title);
	post_content_write(pi->id, new_content, strlen(new_content));
	free(new_content);
	if (content != buf)
		free(content);
	return true;
}

int get_post_mark_raw(fb_time_t stamp, int flag)
{
	int mark = ' ';

	if (flag & POST_FLAG_DIGEST) {
		if (flag & POST_FLAG_MARKED)
			mark = 'b';
		else
			mark = 'g';
	} else if (flag & POST_FLAG_MARKED) {
		mark = 'm';
	}

	if (mark == ' ' && (flag & POST_FLAG_WATER))
		mark = 'w';

	if (brc_unread(stamp)) {
		if (mark == ' ')
			mark = DEFINE(DEF_NOT_N_MASK) ? '+' : 'N';
		else
			mark = toupper(mark);
	}

	return mark;
}

int get_post_mark(const post_info_t *p)
{
	return get_post_mark_raw(p->stamp, p->flag);
}
