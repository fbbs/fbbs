set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

add_definitions(-D_XOPEN_SOURCE=600 -I../../include)

link_directories(../../lib)

foreach(name averun account newacct horoscope zodiac)
	add_executable(${name} ${name}.c chart.c)
	target_link_libraries(${name} BBS)
endforeach(name)

foreach(name bbstop birthday toppost bbsuptime badbms AnnIndex showbm statBM
		cleanuser)
	add_executable(${name} ${name}.c)
	target_link_libraries(${name} BBS)
endforeach(name)

install(TARGETS averun account newacct horoscope zodiac bbstop birthday
toppost bbsuptime badbms AnnIndex showbm statBM cleanuser
RUNTIME DESTINATION tools)
