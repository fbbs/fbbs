#ifndef FB_SCHEMA_H
#define FB_SCHEMA_H

#include <inttypes.h>
#include <stdint.h>

#define PRIdUID  PRId32
#define DBIdUID  "d"
typedef int32_t user_id_t;
#define db_get_user_id(res, row, col)  db_get_integer(res, row, col)

#endif // FB_SCHEMA_H
