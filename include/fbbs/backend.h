#ifndef FB_BACKEND_H
#define FB_BACKEND_H

#include "fbbs/mdbi.h"
#include "fbbs/parcel.h"

#define BACKEND_REQUEST_KEY  "request"
#define BACKEND_RESPONSE_KEY  "response"

#define BACKEND_DECLARE(function)  bool backend_##function(parcel_t *parcel_in, parcel_t *parcel_out, int channel)

typedef enum {
	BACKEND_REQUEST_post_new = 1,
	BACKEND_REQUEST_post_delete = 2,
	BACKEND_REQUEST_post_undelete = 3,
	BACKEND_REQUEST_post_set_flag = 4,
	BACKEND_REQUEST_post_alter_title = 5,
} backend_request_e;

typedef bool (*backend_serializer_t)(const void *request, parcel_t *parcel);
typedef bool (*backend_deserializer_t)(parcel_t *parcel, void *response);

extern bool backend_request(const void *req, void *res, backend_serializer_t serializer, backend_deserializer_t deserializer, backend_request_e type);
#define backend_cmd(req, resp, cmd)  backend_request(req, resp, serialize_##cmd, deserialize_##cmd, BACKEND_REQUEST_##cmd)

extern void backend_respond(parcel_t *parcel, int channel);

#endif // FB_BACKEND_H
