set(CMAKE_C_FLAGS "${CMAKE_C_CFLAGS} -std=c99 -pedantic -Wno-implicit")

add_definitions(-D_XOPEN_SOURCE=500 -DLINUX -DTERMIOS -DSHOW_IDLE_TIME -DDLM
	-DBACKUP_RANGE_DELETE)

link_directories(../lib)

add_library(fbcmn STATIC announce.c bbs.c bcache.c bm.c boards.c chat.c
	comm_lists.c delete.c deny.c edit.c fileshm.c goodbye.c help.c list.c
	mail.c maintain.c more.c namecomplete.c postheader.c read.c register.c
	screen.c sendmsg.c stuff.c talk.c userinfo.c vote.c xyz.c readabc.c five.c
	convert.c bbsnet.c users.c)
target_link_libraries(fbcmn BBS)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_executable(bbsd bbsd.c io.c main.c)
target_link_libraries(bbsd crypt dl BBS fbcmn)

add_executable(miscd miscd.c io.c main.c)
target_link_libraries(miscd crypt dl BBS fbcmn)

add_executable(chatd station.c)
target_link_libraries(chatd BBS)

add_executable(thread thread.c)
target_link_libraries(thread BBS)

add_library(admintool MODULE admintool.c)

install(TARGETS bbsd miscd chatd thread
	RUNTIME DESTINATION bin)
install(TARGETS admintool
	LIBRARY DESTINATION so)
if(ENABLE_SSH)
	add_definitions(-DSSHBBS)
	link_directories(../lib ../libssh)
	add_executable(sshbbsd bbsd.c io.c main.c)
	target_link_libraries(sshbbsd crypt dl BBS fbssh fbcmn)
	install(TARGETS sshbbsd
		RUNTIME DESTINATION bin)
endif(ENABLE_SSH)
