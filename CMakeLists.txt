cmake_minimum_required(VERSION 2.4)

project(fbbs)
set(fbbs_VERSION_MAJOR 2010)
set(fbbs_VERSION_MINOR 0)

include_directories(include)

include(CheckIncludeFiles)
check_include_files(fcntl.h HAVE_FCNTL_H)
configure_file(include/config.h.cmake include/config.h)

option(ENABLE_SSH "compile ssh module" off)
option(ENABLE_DEBUG "compile debug version" on)
option(ENABLE_WWW "compile www module" on)

set(CMAKE_C_FLAGS "-O2 -pipe -Wall")
if(ENABLE_DEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
endif(ENABLE_DEBUG)

subdirs(lib src)
if(ENABLE_SSH)
	add_subdirectory(libssh)
endif(ENABLE_SSH)
if(ENABLE_WWW)
	add_subdirectory(www)
endif(ENABLE_WWW)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#set(CMAKE_COLOR_MAKEFILE off)
