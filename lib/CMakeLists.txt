set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
add_definitions(${FB_XOPEN_SOURCE_DEFINE} ${FB_PLATFORM_DEFINE} -DTERMIOS)

add_library(fbbs_base SHARED cfg.c convert.c fileio.c hash.c mmap.c mmdecode.c
		pool.c string.c time.c util.c)

add_library(fbbs SHARED board.c boardrc.c brdcache.c log.c mail.c
		status.c pass.c post.c record.c shm.c helper.c sysconf.c ucache.c
		uinfo.c register.c user.c session.c title.c friend.c)
target_link_libraries(fbbs m crypt fbbs_base fbbs_pg hiredis)

install(TARGETS fbbs_base fbbs LIBRARY DESTINATION lib)
