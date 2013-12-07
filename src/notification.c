#include "fbbs/mdbi.h"
#include "fbbs/notification.h"
#include "fbbs/session.h"

typedef struct {
	int replies;
} notification_count_t;

static notification_count_t notification_count;

void notification_clear(void)
{
	notification_count.replies = -1;
}

static int *get_notification_cache(notification_item_e item)
{
	int *caches[] = {
		[NOTIFICATION_REPLIES] = &notification_count.replies,
	};
	return caches[item];
}

static int notification_handle(parcel_t *parcel, notification_item_e item)
{
	switch (item) {
		case NOTIFICATION_REPLIES:
			notification_count.replies = -1;
			return 0;
	}
}

int notification_recv(void)
{
	int ret = -1;
	mdb_res_t *res = mdb_recv();
	if (res) {
		mdb_res_t *real_res = mdb_res_at(res, 2);
		if (real_res) {
			parcel_t parcel;
			size_t size;
			const char *ptr = mdb_string_and_size(real_res, &size);
			parcel_read_new(ptr, size, &parcel);

			notification_item_e item = parcel_read_varint(&parcel);
			ret = notification_handle(&parcel, item);
		}
		mdb_clear(res);
	}
	return ret;
}

int notification_cached_get(notification_item_e item)
{
	user_id_t uid = session_uid();
	if (!uid)
		return -1;

	int *cache = get_notification_cache(item);
	if (cache && *cache >= 0)
		return *cache;

	int val = notification_get(item);

	if (cache)
		*cache = val;
	return val;
}
