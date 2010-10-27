cmake_minimum_required(VERSION 2.4)

project(fbbs)
set(fbbs_VERSION_MAJOR 0)
set(fbbs_VERSION_MINOR 1)
set(fbbs_VERSION_PATCH 0)

include_directories(include)

option(ENABLE_SSH "compile ssh module" off)
option(ENABLE_DEBUG "compile debug version" on)
option(ENABLE_WWW "compile www module" off)

set(CMAKE_C_FLAGS "-O2 -pipe -Wall")
if(ENABLE_DEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
endif(ENABLE_DEBUG)

set(CMAKE_INSTALL_PREFIX "/home/bbs")

add_subdirectory(lib)
add_subdirectory(pg)
add_subdirectory(src)
if(ENABLE_WWW)
	add_subdirectory(www)
endif(ENABLE_WWW)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
