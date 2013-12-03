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

/** 回复提醒计数 @mdb_hash **/
#define NOTIFICATION_KEY_REPLIES "new_replies"

static const char *get_notification_key(notification_item_e item)
{
	const char *keys[] = {
		[NOTIFICATION_REPLIES] = NOTIFICATION_KEY_REPLIES,
	};
	return keys[item];
}

static int *get_notification_cache(notification_item_e item)
{
	int *caches[] = {
		[NOTIFICATION_REPLIES] = &notification_count.replies,
	};
	return caches[item];
}

int notification_incr(user_id_t uid, notification_item_e item)
{
	const char *key = get_notification_key(item);
	if (!key)
		return -1;
	return mdb_integer(-1, "HINCRBY", "%s %"PRIdUID" %d", key, uid, 1);
}

int notification_set(user_id_t uid, notification_item_e item, int val)
{
	const char *key = get_notification_key(item);
	if (!key)
		return -1;
	return mdb_integer(-1, "HSET", "%s %"PRIdUID" %d", key, uid, val);
}

void notification_invalidate(notification_item_e item)
{
	int *cache = get_notification_cache(item);
	if (cache)
		*cache = -1;
}

int notification_get(notification_item_e item)
{
	user_id_t uid = session_uid();
	if (!uid)
		return -1;

	int *cache = get_notification_cache(item);
	if (cache && *cache >= 0)
		return *cache;

	const char *key = get_notification_key(item);
	if (!key)
		return -1;
	int val = mdb_integer(-1, "HGET", "%s %"PRIdUID, key, uid);
	if (cache)
		*cache = val;

	return val;
}
