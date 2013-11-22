#include "fbbs/backend.h"
#include "fbbs/helper.h"
#include "fbbs/mdbi.h"
#include "fbbs/parcel.h"

extern bool post_new(parcel_t *, parcel_t *, int);

#define ENTRY(handler)  [BACKEND_REQUEST_##handler] = handler

typedef bool (*handler_t)(parcel_t *parcel_in, parcel_t *parcel_out,
		int channel);

static const handler_t handlers[] = {
	ENTRY(post_new),
};

void backend_respond(parcel_t *parcel, int channel)
{
	if (channel <= 0)
		return;
	mdb_cmd_safe("LPUSH", "%s_%d %b", BACKEND_RESPONSE_KEY, channel,
			parcel, parcel_size(parcel));
}

static void backend_respond_error(parcel_t *parcel, int channel)
{
	parcel_clear(parcel);
	parcel_put(bool, false);
	backend_respond(parcel, channel);
}

int main(int argc, char **argv)
{
	initialize_environment(INIT_MDB | INIT_DB | INIT_CONV);

	while (1) {
		mdb_res_t *res = mdb_res("BLPOP", BACKEND_REQUEST_KEY);
		if (!res)
			return 0;

		bool ok = false;
		int type = 0, channel = 0;
		parcel_t parcel_out;
		parcel_new(&parcel_out);

		size_t size;
		const char *ptr = mdb_string_and_size(res, &size);
		if (ptr) {
			parcel_t parcel_in;
			parcel_read_new(ptr, size, &parcel_in);
			type = parcel_read_varint(&parcel_in);
			channel = parcel_read_varint(&parcel_in);

			if (parcel_ok(&parcel_in) && type >= 0
					&& type < ARRAY_SIZE(handlers)) {
				handler_t handler = handlers[type];
				if (handler) {
					parcel_write_bool(&parcel_out, true);
					parcel_write_varint(&parcel_out, type);

					ok = handler(&parcel_in, &parcel_out, channel);
				}
			}

		}
		mdb_clear(res);

		if (!ok)
			backend_respond_error(&parcel_out, channel);
		parcel_free(&parcel_out);
	}
	return 0;
}
