#ifndef FB_BACKEND_H
#define FB_BACKEND_H

#include "fbbs/mdbi.h"
#include "fbbs/parcel.h"

#define BACKEND_REQUEST_KEY  "request"
#define BACKEND_RESPONSE_KEY  "response"

typedef enum {
	BACKEND_REQUEST_post_new = 1,
	BACKEND_REQUEST_post_delete = 2,
	BACKEND_REQUEST_post_undelete = 3,
} backend_request_e;

typedef bool (*backend_serializer_t)(const void *request, parcel_t *parcel);
typedef bool (*backend_deserializer_t)(parcel_t *parcel, void *response);

extern mdb_res_t *backend_request(const void *req, void *res, backend_serializer_t serializer, backend_deserializer_t deserializer, backend_request_e type);
#define backend_cmd(req, res, cmd)  backend_request(req, res, serialize_##cmd, deserialize_##cmd, BACKEND_REQUEST_##cmd)

extern void backend_respond(parcel_t *parcel, int channel);

#endif // FB_BACKEND_H
