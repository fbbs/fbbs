set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic")
add_definitions(-D_XOPEN_SOURCE=500 -DLINUX -DTERMIOS)

add_library(fbbs_base SHARED cfg.c convert.c fileio.c hash.c mmap.c mmdecode.c
		pool.c string.c time.c util.c)

add_library(fbbs STATIC board.c boardrc.c brdcache.c goodbrd.c log.c mail.c
		modetype.c pass.c post.c record.c shm.c stuffs.c sysconf.c ucache.c
		uinfo.c register.c user.c session.c)
target_link_libraries(fbbs m fbbs_base fbbs_pg)
