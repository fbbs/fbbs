set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")

add_definitions(${FB_XOPEN_SOURCE_DEFINE} ${FB_PLATFORM_DEFINE} -DDLM)

link_directories(/usr/local/lib ../lib)

add_library(fbcmn STATIC announce.c bbs.c bm.c board.c
	comm_lists.c delete.c deny.c edit.c fileshm.c goodbye.c list.c
	mail.c maintain.c more.c namecomplete.c postheader.c read.c register.c
	screen.c sendmsg.c talk.c userinfo.c vote.c xyz.c 
	bbsnet.c online.c prop.c autocomplete.c friend.c plist.c
	alist.c active_board.c tui.c editor.c)
target_link_libraries(fbcmn fbbs)

add_executable(bbsd bbsd.c terminal.c main.c)
target_link_libraries(bbsd crypt ${FB_LIBDL_LINK} fbbs fbcmn)

add_executable(miscd miscd.c terminal.c main.c)
target_link_libraries(miscd crypt ${FB_LIBDL_LINK} fbbs fbcmn)

add_library(admintool MODULE admintool.c title_admin.c money_admin.c)

install(TARGETS bbsd miscd RUNTIME DESTINATION bin)
install(TARGETS admintool LIBRARY DESTINATION lib)
if(ENABLE_SSH)
	link_directories(../lib ../libssh)
	add_executable(sshbbsd bbsd.c terminal.c main.c)
	set_target_properties(sshbbsd PROPERTIES COMPILE_FLAGS "-DENABLE_SSH")
	target_link_libraries(sshbbsd crypt ${FB_LIBDL_LINK} fbbs fbssh fbcmn)
	install(TARGETS sshbbsd
		RUNTIME DESTINATION bin)
endif(ENABLE_SSH)
