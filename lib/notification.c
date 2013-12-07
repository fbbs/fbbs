#include "fbbs/mdbi.h"
#include "fbbs/notification.h"
#include "fbbs/session.h"

/** 回复提醒计数 @mdb_hash **/
#define NOTIFICATION_KEY_REPLIES "new_replies"

static const char *get_notification_key(notification_item_e item)
{
	const char *keys[] = {
		[NOTIFICATION_REPLIES] = NOTIFICATION_KEY_REPLIES,
	};
	return keys[item];
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

int notification_get(notification_item_e item)
{
	user_id_t uid = session_uid();
	if (!uid)
		return -1;

	const char *key = get_notification_key(item);
	if (!key)
		return -1;
	return mdb_integer(-1, "HGET", "%s %"PRIdUID, key, uid);
}

bool notification_send_parcel(user_id_t uid, const parcel_t *parcel)
{
	return mdb_cmd("PUBLISH", "n:%"PRIdUID" %b", uid, parcel->ptr,
			parcel_size(parcel));
}

bool notification_send(user_id_t uid, notification_item_e item)
{
	parcel_t parcel;
	parcel_new(&parcel);
	parcel_write_varint(&parcel, item);

	bool ok = false;
	if (parcel_ok(&parcel))
		ok = notification_send_parcel(uid, &parcel);

	parcel_free(&parcel);

	if (ok)
		notification_incr(uid, item);
	return ok;
}

bool notification_subscribe(void)
{
	return mdb_cmd("SUBSCRIBE", "n:%"PRIdUID, session_uid());
}
