#include "bbs.h"
#include "fbbs/backend.h"
#include "fbbs/cfg.h"
#include "fbbs/fileio.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "s11n/backend_post.h"

static post_id_t insert_post(const backend_request_post_new_t *req)
{
	post_id_t id = 0;
	db_res_t *r1 = db_query("INSERT INTO post.recent (reply_id, thread_id,"
			" user_id, user_id_replied, real_user_id, user_name, board_id,"
			" board_name, marked, locked, attachment, title) VALUES"
			" (%"DBIdPID", %"DBIdPID", %"DBIdUID", %"DBIdUID", %"DBIdUID","
			" %s, %d, %s, %b, %b, %b, %s) RETURNING id", req->reply_id,
			req->thread_id, req->hide_user_id ? 0 : req->user_id,
			req->user_id_replied, req->user_id, req->user_name, req->board_id,
			req->board_name, req->marked, req->locked, false, req->title);
	if (r1 && db_res_rows(r1) == 1) {
		id = db_get_post_id(r1, 0, 0);
		if (id) {
			db_res_t *r2 = db_cmd("INSERT INTO post.content"
					" (post_id, content) VALUES (%"DBIdPID", %s)",
					id, req->content);
			db_clear(r2);
		}
	}
	db_clear(r1);
	return id;
}

static void adjust_user_post_count(const char *uname, int delta)
{
	struct userec urec;
	int unum = searchuser(uname);
	getuserbyuid(&urec, unum);
	urec.numposts += delta;
	substitut_record(NULL, &urec, sizeof(urec), unum);
}

static int _insert_reply_record(const char *table_name,
		const backend_request_post_new_t *req, post_id_t post_id,
		user_id_t user_id)
{
	query_t *q = query_new(0);
	query_sappend(q, "INSERT INTO", table_name);
	query_append(q, "(post_id, reply_id, thread_id, user_id_replied, user_id,"
			"user_name, board_id, board_name, title, is_read)"
			" VALUES (%"DBIdPID", %"DBIdPID", %"DBIdPID", %d, %d,"
			" %s, %d, %s, %s, FALSE)",
			post_id, req->reply_id ? req->reply_id : post_id,
			req->thread_id ? req->thread_id : post_id, user_id,
			req->hide_user_id ? 0 : req->user_id, req->user_name,
			req->board_id, req->board_name, req->title);

	db_res_t *res = query_cmd(q);
	int rows = db_cmd_rows(res);
	db_clear(res);
	return rows;
}

static void insert_reply_record(const backend_request_post_new_t *req,
		post_id_t post_id, user_id_t user_id)
{
	char table_name[64];
	post_reply_table_name(user_id, table_name, sizeof(table_name));
	if (_insert_reply_record(table_name, req, post_id, user_id) > 0)
		post_reply_incr_count(user_id, 1);
}

static void notify_new_reply(const backend_request_post_new_t *req,
		const board_t *board, post_id_t post_id)
{
	struct userec urec;
	if (getuserec(req->user_name, &urec)
			&& user_has_read_perm(&urec, board)) {
		insert_reply_record(req, post_id, req->user_id_replied);
	}
}

static int insert_mention_record(const backend_request_post_new_t *req,
		post_id_t post_id, user_id_t user_id)
{
	char table_name[64];
	post_mention_table_name(user_id, table_name, sizeof(table_name));
	return _insert_reply_record(table_name, req, post_id, user_id);
}

static int process_mention(const char *user_name,
		char (*user_names)[IDLEN + 1], size_t size, int count,
		const backend_request_post_new_t *req, const board_t *board,
		post_id_t post_id)
{
	for (size_t i = 0; i < count; ++i) {
		if (strcaseeq(user_name, user_names[i]))
			return 0;
	}

	if (count < size) {
		strlcpy(user_names[count], user_name, sizeof(user_names[count]));
		++count;

		struct userec urec;
		user_id_t user_id;
		if (getuserec(user_name, &urec)
				&& user_has_read_perm(&urec, board)
				&& (user_id = get_user_id(user_name)) > 0
				&& user_id != req->user_id) {
			if (insert_mention_record(req, post_id, user_id) > 0)
				post_mention_incr_count(user_id, 1);
		}
	}
	return 1;
}

static const char *next_mention(const char *begin, const char *end,
		char *user_name, size_t size)
{
	*user_name = '\0';
	bool mention = false;
	const char *at = NULL, *ptr;
	for (ptr = begin; ptr < end; ++ptr) {
		if (mention) {
			if (!isalpha(*ptr)) {
				if (ptr - at >= 2 && ptr - at < size) {
					strlcpy(user_name, at, ptr - at + 1);
					return ptr;
				}
				mention = false;
				at = NULL;
			}
		}
		if (!mention) {
			if (*ptr == '@') {
				mention = true;
				at = ptr + 1;
			}
		}
	}
	return ptr;
}

static int scan_for_mentions(const backend_request_post_new_t *req,
		const board_t *board, post_id_t post_id)
{
	const char *begin = req->content;
	if (strneq(req->title, "Re: ", 4))
		begin = strstr(begin, "\n\n");
	if (!begin)
		return 0;

	// 跳过签名档
	const char *end = strstr(begin, "\n--\n");
	if (end)
		++end;
	else
		end = req->content + strlen(req->content);

	int count = 0;
	char user_names[POST_MENTION_LIMIT][IDLEN + 1] = { { '\0' } };

	const char *line_end;
	while (begin < end && (line_end = get_line_end(begin, end)) <= end) {
		// 跳过引用行
		if (line_end - begin >= 2 && *begin == ':' && begin[1] == ' ') {
			begin = line_end;
			continue;
		}

		const char *ptr = begin;
		while (ptr < line_end) {
			char user_name[IDLEN + 1];
			ptr = next_mention(ptr, line_end, user_name, sizeof(user_name));
			if (*user_name) {
				count += process_mention(user_name, user_names,
						ARRAY_SIZE(user_names), count, req, board, post_id);
			}
		}
		begin = line_end;
	}
	return count;
}

BACKEND_DECLARE(post_new)
{
	backend_request_post_new_t req;
	if (!deserialize_post_new(parcel_in, &req))
		return false;

	post_id_t post_id = insert_post(&req);
	if (post_id) {
		backend_response_post_new_t resp = { .id = post_id };
		serialize_post_new(&resp, parcel_out);
		backend_respond(parcel_out, channel);

		fb_time_t stamp = post_stamp(post_id);
		set_last_post_time(req.board_id, stamp);
		post_record_invalidity_change(req.board_id, 1);

		board_t board;
		if (get_board_by_bid(req.board_id, &board)) {
			if (!board_is_junk(&board))
				adjust_user_post_count(req.user_name, 1);

			if (req.user_id != req.user_id_replied)
				notify_new_reply(&req, &board, post_id);

			scan_for_mentions(&req, &board, post_id);
		}
		return true;
	}
	return false;
}

static void build_post_filter(query_t *q, const post_filter_t *f)
{
	query_where(q, "TRUE");
	if (f->bid && is_deleted(f->type))
		query_and(q, "board_id = %d", f->bid);
	if (f->flag & POST_FLAG_DIGEST)
		query_and(q, "digest");
	if (f->flag & POST_FLAG_MARKED)
		query_and(q, "marked");
	if (f->flag & POST_FLAG_WATER)
		query_and(q, "water");
	if (f->uid)
		query_and(q, "user_id = %"DBIdUID, f->uid);
	if (*f->utf8_keyword)
		query_and(q, "title ILIKE '%%' || %s || '%%'", f->utf8_keyword);
	if (f->type == POST_LIST_TOPIC)
		query_and(q, "id = thread_id");

	if (f->type == POST_LIST_THREAD) {
		if (f->min && f->tid) {
			query_and(q, "(thread_id = %"DBIdPID" AND id >= %"DBIdPID
					" OR thread_id > %"DBIdPID")", f->tid, f->min, f->tid);
		}
		if (f->max && f->tid) {
			query_and(q, "(thread_id = %"DBIdPID" AND id <= %"DBIdPID
					" OR thread_id < %"DBIdPID")", f->tid, f->max, f->tid);
		}
	} else {
		if (f->min)
			query_and(q, "id >= %"DBIdPID, f->min);
		if (f->max)
			query_and(q, "id <= %"DBIdPID, f->max);
		if (f->tid)
			query_and(q, "thread_id = %"DBIdPID, f->tid);
	}

	if (f->type == POST_LIST_TRASH)
		query_and(q, "bm_visible");
	if (f->type == POST_LIST_JUNK)
		query_and(q, "NOT bm_visible");
}

static const char *table_name(const post_filter_t *filter)
{
	if (is_deleted(filter->type))
		return "post.deleted";
	else
		return "post.recent";
}

static bool remove_cached_content(post_id_t post_id)
{
	if (post_id <= 0)
		return false;
	char old[HOMELEN], new_[HOMELEN];
	post_content_cache_filename(post_id, old, sizeof(old));
	post_content_deleted_filename(post_id, new_, sizeof(new_));
	if (rename(old, new_) == 0)
		return true;
	if (errno == ENOENT)
		return file_append(new_, "") > 0;
	return false;
}

static int _backend_post_delete(const backend_request_post_delete_t *req)
{
	board_t board;
	if (req->filter->bid && !get_board_by_bid(req->filter->bid, &board))
		return 0;

	bool decrease = !(req->filter->bid && board_is_junk(&board));

	query_t *q = query_new(0);
	query_append(q, "WITH rows AS (DELETE FROM post.recent");
	build_post_filter(q, req->filter);

	if (!req->force)
		query_and(q, "NOT marked");
	query_and(q, "NOT sticky");

	query_append(q, "RETURNING " POST_TABLE_FIELDS ")");
	query_append(q, "INSERT INTO post.deleted ("
			POST_TABLE_FIELDS "," POST_TABLE_DELETED_FIELDS ")");
	query_append(q, "SELECT " POST_TABLE_FIELDS ","
			" current_timestamp, %"DBIdUID", %s, %b AND (water OR %b), %b"
			" FROM rows", req->user_id, req->user_name, decrease, req->junk,
			req->bm_visible);
	query_append(q, "RETURNING id, user_id, user_name, junk");

	db_res_t *res = query_exec(q);
	int rows = 0;
	if (res) {
		rows = db_res_rows(res);
		for (int i = 0; i < rows; ++i) {
			user_id_t user_id = db_get_user_id(res, i, 1);
			if (user_id && db_get_bool(res, i, 3)) {
				const char *user_name = db_get_value(res, i, 2);
				adjust_user_post_count(user_name, -1);
			}

			remove_cached_content(db_get_post_id(res, i, 0));
		}
		if (rows > 0)
			post_record_invalidity_change(req->filter->bid, 1);
	}
	db_clear(res);
	return rows;
}

BACKEND_DECLARE(post_delete)
{
	post_filter_t filter;
	backend_request_post_delete_t req = { .filter = &filter };
	if (!deserialize_post_delete(parcel_in, &req))
		return false;

	backend_response_post_delete_t resp;
	resp.deleted = _backend_post_delete(&req);
	serialize_post_delete(&resp, parcel_out);
	backend_respond(parcel_out, channel);
	return true;
}

static int _backend_post_undelete(const backend_request_post_undelete_t *req)
{
	query_t *q = query_new(0);
	query_select(q, "id, user_id, user_name, junk FROM post.deleted");
	build_post_filter(q, req->filter);

	db_res_t *res = query_exec(q);
	if (res) {
		for (int i = db_res_rows(res) - 1; i >= 0; --i) {
			user_id_t user_id = db_get_user_id(res, i, 1);
			if (user_id && db_get_bool(res, i, 3)) {
				const char *user_name = db_get_value(res, i, 2);
				adjust_user_post_count(user_name, 1);
			}

			post_id_t post_id = db_get_post_id(res, i, 0);
			if (post_id > 0) {
				char file[HOMELEN];
				post_content_deleted_filename(post_id, file, sizeof(file));
				unlink(file);
			}
		}
	}
	db_clear(res);

	q = query_new(0);
	query_append(q, "WITH rows AS (DELETE FROM post.deleted");
	build_post_filter(q, req->filter);
	// TODO 站务垃圾箱和版主垃圾箱?
	query_append(q, "RETURNING " POST_TABLE_FIELDS ")");
	query_append(q, "INSERT INTO post.recent (" POST_TABLE_FIELDS ")");
	query_select(q, POST_TABLE_FIELDS);
	query_from(q, "rows");

	res = query_cmd(q);
	int rows = res ? db_cmd_rows(res) : 0;
	db_clear(res);

	if (rows)
		post_record_invalidity_change(req->filter->bid, 1);
	return rows;
}

BACKEND_DECLARE(post_undelete)
{
	post_filter_t filter;
	backend_request_post_undelete_t req = { .filter = &filter };
	if (!deserialize_post_undelete(parcel_in, &req))
		return false;

	backend_response_post_undelete_t resp;
	resp.undeleted = _backend_post_undelete(&req);
	serialize_post_undelete(&resp, parcel_out);
	backend_respond(parcel_out, channel);
	return true;
}

static const char *flag_field(post_flag_e flag)
{
	switch (flag) {
		case POST_FLAG_DIGEST:
			return "digest";
		case POST_FLAG_MARKED:
			return "marked";
		case POST_FLAG_LOCKED:
			return "locked";
		case POST_FLAG_IMPORT:
			return "imported";
		case POST_FLAG_WATER:
			return "water";
		case POST_FLAG_STICKY:
			return "sticky";
		default:
			return "attachment";
	}
}

static int _backend_post_set_flag(const backend_request_post_set_flag_t *req)
{
	if (!req)
		return 0;

	query_t *q = query_new(0);
	if (!q)
		return 0;

	query_update(q, table_name(req->filter));

	const char *field = flag_field(req->flag);
	query_set(q, field);
	if (req->toggle) {
		query_sappend(q, "= NOT", field);
	} else {
		query_append(q, "= %b", req->set);
	}
	build_post_filter(q, req->filter);

	db_res_t *res = query_cmd(q);
	int rows = res ? db_cmd_rows(res) : 0;
	db_clear(res);

	if (rows > 0)
		post_record_invalidity_change(req->filter->bid, 1);
	return rows;
}

BACKEND_DECLARE(post_set_flag)
{
	post_filter_t filter;
	backend_request_post_set_flag_t req = { .filter = &filter };
	if (!deserialize_post_set_flag(parcel_in, &req))
		return false;

	backend_response_post_set_flag_t resp;
	resp.affected = _backend_post_set_flag(&req);

	if (resp.affected)
		post_record_invalidity_change(filter.bid, 1);

	serialize_post_set_flag(&resp, parcel_out);
	backend_respond(parcel_out, channel);
	return true;
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
	memcpy(p, l2_end, end - l2_end);
	s[len] = '\0';
	return s;
}

static bool _backend_post_alter_title(
		const backend_request_post_alter_title_t *req)
{
	if (!req)
		return false;

	post_filter_t filter = {
		.bid = req->board_id,
		.min = req->post_id,
		.max = req->post_id,
	};

	query_t *q = query_new(0);
	query_update(q, "post.recent");
	query_set(q, "title = %s", req->title);
	build_post_filter(q, &filter);

	db_res_t *res = query_cmd(q);
	bool ok = res && db_cmd_rows(res) > 0;
	db_clear(res);
	if (!ok)
		return ok;

	post_record_invalidity_change(req->board_id, 1);

	char *content = post_content_get(req->post_id, true);
	if (!content)
		return false;

	char *new_content = replace_content_title(content, strlen(content),
			req->title);

	if (new_content)
		ok = post_content_set(req->post_id, new_content);

	free(new_content);
	free(content);
	return ok;
}

BACKEND_DECLARE(post_alter_title)
{
	backend_request_post_alter_title_t req;
	if (!deserialize_post_alter_title(parcel_in, &req))
		return false;

	backend_response_post_alter_title_t resp;
	resp.ok = _backend_post_alter_title(&req);

	serialize_post_alter_title(&resp, parcel_out);
	backend_respond(parcel_out, channel);
	return true;
}
