add_definitions(-D_XOPEN_SOURCE=600 -I../include)

link_directories(../lib)

set(UTILS1 averun account newacct horoscope zodiac)
set(UTILS2 bbstop birthday toppost bbsuptime badbms AnnIndex showbm statBM
	cleanuser clean_session)

foreach(name ${UTILS1})
	add_executable(${name} ${name}.c chart.c)
	target_link_libraries(${name} fbbs)
endforeach(name)

foreach(name ${UTILS2})
	add_executable(${name} ${name}.c)
	target_link_libraries(${name} fbbs)
endforeach(name)

install(TARGETS ${UTILS1} ${UTILS2} RUNTIME DESTINATION tools)
