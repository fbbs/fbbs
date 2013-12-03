#ifndef FB_NOTIFICATION_H
#define FB_NOTIFICATION_H

#include "fbbs/user.h"

typedef enum {
	NOTIFICATION_REPLIES,
} notification_item_e;

extern void notification_clear(void);
extern int notification_incr(user_id_t uid, notification_item_e item);
extern int notification_set(user_id_t uid, notification_item_e item, int val);
extern void notification_invalidate(notification_item_e item);
extern int notification_get(notification_item_e item);

#endif // FB_NOTIFICATION_H
