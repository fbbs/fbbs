set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -Wno-implicit")

add_definitions(${FB_XOPEN_SOURCE_DEFINE} -DLINUX -DTERMIOS -DSHOW_IDLE_TIME
		-DDLM -DBACKUP_RANGE_DELETE)

link_directories(/usr/local/lib ../lib)

add_library(fbcmn STATIC announce.c bbs.c bcache.c bm.c boards.c chat.c
	comm_lists.c delete.c deny.c edit.c fileshm.c goodbye.c help.c list.c
	mail.c maintain.c more.c namecomplete.c postheader.c read.c register.c
	screen.c sendmsg.c stuff.c talk.c userinfo.c vote.c xyz.c readabc.c
	convert.c bbsnet.c users.c)
target_link_libraries(fbcmn fbbs)

add_executable(bbsd bbsd.c io.c main.c)
target_link_libraries(bbsd crypt ${FB_LIBDL_LINK} fbbs fbcmn)

add_executable(miscd miscd.c io.c main.c)
target_link_libraries(miscd crypt ${FB_LIBDL_LINK} fbbs fbcmn)

add_executable(chatd station.c)
target_link_libraries(chatd fbbs)

add_executable(thread thread.c)
target_link_libraries(thread fbbs)

add_library(admintool MODULE admintool.c)

install(TARGETS bbsd miscd chatd thread
	RUNTIME DESTINATION bin)
install(TARGETS admintool LIBRARY DESTINATION lib)
if(ENABLE_SSH)
	link_directories(../lib ../libssh)
	add_executable(sshbbsd bbsd.c io.c main.c)
	set_target_properties(sshbbsd PROPERTIES COMPILE_FLAGS "-DENABLE_SSH")
	target_link_libraries(sshbbsd crypt ${FB_LIBDL_LINK} fbbs fbssh fbcmn)
	install(TARGETS sshbbsd
		RUNTIME DESTINATION bin)
endif(ENABLE_SSH)
