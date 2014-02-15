#include "bbs.h"
#include "fbbs/backend.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/notification.h"
#include "fbbs/post.h"
#include "fbbs/string.h"
#include "s11n/backend_post.h"

#define POST_TABLE_FIELDS \
	"id, reply_id, thread_id, user_id, real_user_id, user_name, board_id," \
	" digest, marked, locked, imported, water, attachment, title"

#define POST_TABLE_DELETED_FIELDS \
	"delete_stamp, eraser_id, eraser_name, junk, bm_visible"

static void set_board_post_count(int bid, int count)
{
	mdb_integer(0, "HSET", BOARD_POST_COUNT_KEY " %d %d", bid, count);
}

static post_id_t insert_post(const backend_request_post_new_t *req)
{
	post_id_t id = 0;
	db_res_t *r1 = db_query("INSERT INTO posts.recent (reply_id, thread_id,"
			" user_id, user_name, board_id, marked, locked, attachment, title)"
			" VALUES (%"DBIdPID", %"DBIdPID", %"DBIdUID", %s, %d, %b, %b, %b,"
			" %s) RETURNING id", req->reid, req->tid,
			req->hide_uid ? 0 : req->uid, req->uname, req->bid, req->marked,
			req->locked, false, req->title);
	if (r1 && db_res_rows(r1) == 1) {
		id = db_get_post_id(r1, 0, 0);
		if (id) {
			db_res_t *r2 = db_cmd("INSERT INTO posts.content"
					" (post_id, content) VALUES (%"DBIdPID", %s)",
					id, req->content);
			db_clear(r2);
		}
	}
	db_clear(r1);
	return id;
}
#if 0
static bool append_notification_file(const char *uname, user_id_t uid,
		const board_t *board, const post_index_board_t *pib,
		const char *basename)
{
	struct userec user;
	if (!getuserec(uname, &user) || !user_has_read_perm(&user, board))
		return false;

	char file[HOMELEN];
	sethomefile(file, uname, basename);

	record_t record;
	if (record_open(file, post_index_cmp, sizeof(*pib), RECORD_WRITE, &record)
			< 0)
		return false;
	bool ok = record_append_locked(&record, pib, 1) >= 0;
	record_close(&record);

	notification_send(uid, NOTIFICATION_REPLIES);
	return ok;
}

static void send_reply_notification(const post_index_board_t *pib,
		const char *uname, user_id_t uid, const board_t *board)
{
	if (uid && uid != pib->uid) {
		append_notification_file(uname, uid, board, pib, POST_REPLIES_FILE);
	}
}
#endif

static void adjust_user_post_count(const char *uname, int delta)
{
	struct userec urec;
	int unum = searchuser(uname);
	getuserbyuid(&urec, unum);
	urec.numposts += delta;
	substitut_record(NULL, &urec, sizeof(urec), unum);
}

BACKEND_DECLARE(post_new)
{
	backend_request_post_new_t req;
	if (!deserialize_post_new(parcel_in, &req))
		return false;

	post_id_t id = insert_post(&req);
	if (id) {
		backend_response_post_new_t resp = { .id = id };
		serialize_post_new(&resp, parcel_out);
		backend_respond(parcel_out, channel);

		fb_time_t stamp = post_stamp_from_id(id);
		set_last_post_time(req.bid, stamp);
		post_cache_invalidity_change(req.bid, 1);

		// TODO: 不总是要加文章数的..
		adjust_user_post_count(req.uname, 1);

#if 0
		board_t board;
		if (get_board_by_bid(req.bid, &board)) {
			send_reply_notification(&pib, req.uname_replied, req.uid_replied,
					&board);
		}
		return true;
#endif
	}
	return id;
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
		return "posts.deleted";
	else
		return "posts.recent";
}

static int _backend_post_delete(const backend_request_post_delete_t *req)
{
	board_t board;
	if (req->filter->bid && !get_board_by_bid(req->filter->bid, &board))
		return 0;

	bool decrease = !(req->filter->bid && is_junk_board(&board));

	query_t *q = query_new(0);
	query_append(q, "WITH rows AS (DELETE FROM posts.recent");
	build_post_filter(q, req->filter);

	if (!req->force)
		query_and(q, "NOT marked");
	query_and(q, "NOT sticky");

	query_append(q, "RETURNING " POST_TABLE_FIELDS ")");
	query_append(q, "INSERT INTO posts.deleted ("
			POST_TABLE_FIELDS "," POST_TABLE_DELETED_FIELDS ")");
	query_append(q, "SELECT " POST_TABLE_FIELDS ","
			" current_timestamp, %"DBIdUID", %s, %b AND (water OR %b), %b"
			" FROM rows", req->user_id, req->user_name, decrease, req->junk,
			req->bm_visible);
	query_append(q, "RETURNING user_id, user_name, junk");

	db_res_t *res = query_exec(q);
	int rows = 0;
	if (res) {
		rows = db_res_rows(res);
		for (int i = 0; i < rows; ++i) {
			user_id_t uid = db_get_user_id(res, i, 0);
			if (uid && db_get_bool(res, i, 2)) {
				const char *user_name = db_get_value(res, i, 1);
				adjust_user_post_count(user_name, -1);
			}
		}
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
	query_select(q, "user_id, user_name, junk FROM posts.deleted");
	build_post_filter(q, req->filter);

	db_res_t *res = query_exec(q);
	if (res) {
		for (int i = db_res_rows(res) - 1; i >= 0; --i) {
			user_id_t user_id = db_get_user_id(res, i, 0);
			if (user_id && db_get_bool(res, i, 2)) {
				const char *user_name = db_get_value(res, i, 1);
				adjust_user_post_count(user_name, 1);
			}
		}
	}
	db_clear(res);

	q = query_new(0);
	query_append(q, "WITH rows AS (DELETE FROM posts.deleted");
	build_post_filter(q, req->filter);
	// TODO 站务垃圾箱和版主垃圾箱?
	query_append(q, "RETURNING " POST_TABLE_FIELDS ")");
	query_append(q, "INSERT INTO posts.recent (" POST_TABLE_FIELDS ")");
	query_select(q, POST_TABLE_FIELDS);
	query_from(q, "rows");

	res = query_cmd(q);
	int rows = res ? db_cmd_rows(res) : 0;
	db_clear(res);
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
		post_cache_invalidity_change(filter.bid, 1);

	serialize_post_set_flag(&resp, parcel_out);
	backend_respond(parcel_out, channel);
	return true;
}
