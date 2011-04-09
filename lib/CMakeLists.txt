set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic")
add_definitions(-D_XOPEN_SOURCE=500 -DLINUX -DTERMIOS)

add_library(BBS STATIC board.c boardrc.c brdcache.c fileio.c goodbrd.c log.c
	mail.c mmap.c mmdecode.c modetype.c pass.c record.c post.c shm.c string.c
	stuffs.c sysconf.c time.c ucache.c uinfo.c hash.c register.c util.c cfg.c
	pool.c convert.c)
target_link_libraries(BBS m)
