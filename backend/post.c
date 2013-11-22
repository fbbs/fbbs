#include "bbs.h"
#include "fbbs/backend.h"
#include "fbbs/mdbi.h"
#include "fbbs/post.h"
#include "fbbs/string.h"

// TODO: move here
extern void set_board_post_count(int bid, int count);

static bool deserialize_post_new(parcel_t *parcel,
		backend_request_post_new_t *req)
{
	req->reid = parcel_get(post_id);
	req->tid = parcel_get(post_id);
	req->title = parcel_get(string);
	req->uname = parcel_get(string);
	req->content = parcel_get(string);
	req->bid = parcel_get(int);
	req->marked = parcel_get(bool);
	req->locked = parcel_get(bool);
	return parcel_ok(parcel);
}

static post_id_t next_post_id(void)
{
	return mdb_integer(0, "INCR", POST_ID_KEY);
}

static post_id_t post_insert(backend_request_post_new_t *req)
{
	post_index_t pi = { .id = next_post_id(), };
	if (pi.id) {
		pi.reid_delta = req->reid ? pi.id - req->reid : 0;
		pi.tid_delta = req->tid ? pi.id - req->tid : 0;
		pi.stamp = fb_time();
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

void post_new(parcel_t *parcel)
{
	backend_request_post_new_t req;
	if (!deserialize_post_new(parcel, &req))
		return;

	post_insert(&req);
}
