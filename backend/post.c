#include "bbs.h"
#include "fbbs/backend.h"
#include "fbbs/mdbi.h"
#include "fbbs/post.h"
#include "fbbs/string.h"

#include "s11n/backend_post.h"

// TODO: move here
extern void set_board_post_count(int bid, int count);

static post_id_t next_post_id(void)
{
	return mdb_integer(0, "INCR", POST_ID_KEY);
}

static post_id_t insert_post(const backend_request_post_new_t *req,
		fb_time_t now)
{
	post_index_t pi = { .id = next_post_id(), };
	if (pi.id) {
		pi.reid_delta = req->reid ? pi.id - req->reid : 0;
		pi.tid_delta = req->tid ? pi.id - req->tid : 0;
		pi.stamp = now;
		pi.uid = get_user_id(req->uname);
		pi.flag = (req->marked ? POST_FLAG_MARKED : 0)
				| (req->locked ? POST_FLAG_LOCKED : 0);
		pi.bid = req->bid;
		strlcpy(pi.owner, req->uname, sizeof(pi.owner));
		string_cp(pi.utf8_title, req->title, sizeof(pi.utf8_title));

		post_content_write(pi.id, req->content, strlen(req->content));

		post_index_record_t pir;
		post_index_record_open(&pir);
		if (!post_index_record_update(&pir, &pi))
			pi.id = 0;
		post_index_record_close(&pir);
	}
	if (pi.id) {
		post_index_board_t pib = {
			.id = pi.id, .reid_delta = pi.reid_delta, .tid_delta = pi.tid_delta,
			.uid = pi.uid, .flag = pi.flag, .stamp = pi.stamp, .cstamp = 0,
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

bool post_new(parcel_t *parcel_in, parcel_t *parcel_out, int channel)
{
	backend_request_post_new_t req;
	if (!deserialize_post_new(parcel_in, &req))
		return false;

	fb_time_t now = fb_time();
	post_id_t id = insert_post(&req, now);

	if (id) {
		backend_response_post_new_t resp = { .id = id, .stamp = now };
		serialize_post_new(&resp, parcel_out);
		backend_respond(parcel_out, channel);
		return true;
	}
	return false;
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
//		post_filter_t filter2 = { .offset_min = current + 1 };
//		post_deletion_trigger(&trash, &filter2, &pir, true);
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
