#ifndef FB_FILE_H
#define FB_FILE_H

#include <stdbool.h>

extern bool dashf(const char *file);
extern int fb_flock(int fd, int operation);

#endif // FB_FILE_H
