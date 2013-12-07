#ifndef FB_NOTIFICATION_H
#define FB_NOTIFICATION_H

#include "fbbs/parcel.h"
#include "fbbs/user.h"

typedef enum {
	NOTIFICATION_REPLIES,
} notification_item_e;

extern void notification_clear(void);
extern int notification_incr(user_id_t uid, notification_item_e item);
extern int notification_set(user_id_t uid, notification_item_e item, int val);
extern int notification_get(notification_item_e item);
extern bool notification_send_parcel(user_id_t uid, const parcel_t *parcel);
extern bool notification_send(user_id_t uid, notification_item_e item);
extern bool notification_subscribe(void);

#endif // FB_NOTIFICATION_H
