#include "bbs.h"
#include "fbbs/backend.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/notification.h"
#include "fbbs/post.h"
#include "fbbs/string.h"

#include "s11n/backend_post.h"

static void set_board_post_count(int bid, int count)
{
	mdb_integer(0, "HSET", BOARD_POST_COUNT_KEY " %d %d", bid, count);
}

static post_id_t next_post_id(void)
{
	return mdb_integer(0, "INCR", POST_ID_KEY);
}

static post_id_t insert_post_index(const backend_request_post_new_t *req,
		fb_time_t now, post_index_t *pi)
{
	pi->id = next_post_id();
	if (!pi->id)
		return 0;

	pi->reid_delta = req->reid ? pi->id - req->reid : 0;
	pi->tid_delta = req->tid ? pi->id - req->tid : 0;
	pi->stamp = now;
	pi->uid = get_user_id(req->uname);
	pi->flag = (req->marked ? POST_FLAG_MARKED : 0)
			| (req->locked ? POST_FLAG_LOCKED : 0);
	pi->bid = req->bid;
	strlcpy(pi->owner, req->uname, sizeof(pi->owner));
	string_cp(pi->utf8_title, req->title, sizeof(pi->utf8_title));

	post_index_record_t pir;
	post_index_record_open(&pir);
	if (!post_index_record_update(&pir, pi))
		pi->id = 0;
	post_index_record_close(&pir);
	return pi->id;
}

static post_id_t insert_post_index_board(const post_index_t *pi,
		post_index_board_t *pib)
{
	pib->id = pi->id;
	pib->reid_delta = pi->reid_delta;
	pib->tid_delta = pi->tid_delta;
	pib->uid = pi->uid;
	pib->flag = pi->flag;
	pib->stamp = pi->stamp;
	pib->cstamp = 0;

	post_id_t id = 0;
	record_t record;
	if (post_index_board_open(pi->bid, RECORD_WRITE, &record) < 0)
		return 0;

	if (record_append_locked(&record, &pib, 1) >= 0) {
		id = pi->id;
		set_board_post_count(pi->bid, record_count(&record));
	}
	record_close(&record);
	return id;
}

static post_id_t insert_post(const backend_request_post_new_t *req,
		fb_time_t now, post_index_board_t *pib)
{
	post_index_t pi;
	if (!insert_post_index(req, now, &pi))
		return 0;

	if (post_content_write(pi.id, req->content, strlen(req->content)) <= 0)
		return 0;

	return insert_post_index_board(&pi, pib);
}

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

bool post_new(parcel_t *parcel_in, parcel_t *parcel_out, int channel)
{
	backend_request_post_new_t req;
	if (!deserialize_post_new(parcel_in, &req))
		return false;

	fb_time_t now = fb_time();
	post_index_board_t pib;
	post_id_t id = insert_post(&req, now, &pib);

	if (id) {
		backend_response_post_new_t resp = { .id = id, .stamp = now };
		serialize_post_new(&resp, parcel_out);
		backend_respond(parcel_out, channel);

		board_t board;
		if (get_board_by_bid(req.bid, &board)) {
			send_reply_notification(&pib, req.uname_replied, req.uid_replied,
					&board);
		}
		return true;
	}
	return false;
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

static int delete_posts(const backend_request_post_delete_t *req)
{
	bool decrease = true;
	board_t board;
	if (get_board_by_bid(req->filter->bid, &board) && is_junk_board(&board))
		decrease = false;

	int deleted = 0;

	record_t record, trash;
	if (post_index_board_open(req->filter->bid, RECORD_WRITE, &record) < 0)
		goto e1;
	if (post_index_trash_open(req->filter->bid,
			req->bm_visible ? POST_INDEX_TRASH : POST_INDEX_JUNK, &trash) < 0)
		goto e2;
	post_index_record_t pir;
	post_index_record_open(&pir);

	post_index_trash_insert_t piti = {
		.filter = req->filter, .pir = &pir,
		.trash = &trash, .ename = req->ename,
		.estamp = fb_time(), .junk = req->junk, .decrease = decrease,
	};

	if (record_lock_all(&trash, RECORD_WRLCK) < 0)
		goto e3;
	int current = record_seek(&trash, 0, RECORD_END);

	if (record_lock_all(&record, RECORD_WRLCK) < 0)
		goto e4;
	deleted = record_delete(&record, NULL, 0, post_index_trash_insert, &piti);
	record_lock_all(&record, RECORD_UNLCK);

	if (deleted) {
		set_board_post_count(req->filter->bid, record_count(&record));
		post_filter_t filter2 = { .offset_min = current + 1 };
		post_deletion_trigger(&trash, &filter2, &pir, true);
	}

e4: record_lock_all(&trash, RECORD_UNLCK);
e3: post_index_record_close(&pir);
e2: record_close(&trash);
e1: record_close(&record);
	return deleted;
}

bool post_delete(parcel_t *parcel_in, parcel_t *parcel_out, int channel)
{
	post_filter_t filter;
	backend_request_post_delete_t req = { .filter = &filter };
	if (!deserialize_post_delete(parcel_in, &req))
		return false;

	backend_response_post_delete_t resp;
	resp.deleted = delete_posts(&req);
	serialize_post_delete(&resp, parcel_out);
	backend_respond(parcel_out, channel);
	return true;
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

int undelete_posts(const backend_request_post_undelete_t *req)
{
	record_t record, trash;
	post_index_board_open(req->filter->bid, RECORD_WRITE, &record);
	post_index_trash_open(req->filter->bid,
			req->bm_visible ? POST_INDEX_TRASH : POST_INDEX_JUNK, &trash);
	post_index_record_t pir;
	post_index_record_open(&pir);

	record_lock_all(&trash, RECORD_WRLCK);
	int undeleted = post_deletion_trigger(&trash, req->filter, &pir, false);
	post_index_board_t *buf = malloc(undeleted * sizeof(post_index_board_t));
	post_undeletion_callback_t puc = {
		.pir = &pir, .filter = req->filter,
		.buf = buf, .count = 0, .max = undeleted,
	};

	record_lock_all(&record, RECORD_WRLCK);
	record_delete(&trash, NULL, 0, post_undeletion_callback, &puc);
	record_merge(&record, puc.buf, puc.count);
	record_lock_all(&record, RECORD_UNLCK);
	set_board_post_count(req->filter->bid, record_count(&record));

	record_lock_all(&trash, RECORD_UNLCK);
	free(buf);
	post_index_record_close(&pir);
	record_close(&trash);
	record_close(&record);
	return undeleted;
}

bool post_undelete(parcel_t *parcel_in, parcel_t *parcel_out, int channel)
{
	post_filter_t filter;
	backend_request_post_undelete_t req = { .filter = &filter };
	if (!deserialize_post_undelete(parcel_in, &req))
		return false;

	backend_response_post_undelete_t resp;
	resp.undeleted = undelete_posts(&req);
	serialize_post_undelete(&resp, parcel_out);
	backend_respond(parcel_out, channel);
	return true;
}
