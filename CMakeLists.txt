cmake_minimum_required(VERSION 3.14.0)
project(garbage_collcetor VERSION 0.1.0 LANGUAGES C)

include_directories(include src)

add_subdirectory(tests)

file(GLOB_RECURSE SOURCES "src/*.c")

if(APPLE)
    file(GLOB_RECURSE PLATFORM_SOURCES "platform/apple/*.c")
elseif(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        file(GLOB_RECURSE PLATFORM_SOURCES "platform/linux/*.c")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD" OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
        file(GLOB_RECURSE PLATFORM_SOURCES "platform/bsd/*.c")
    endif()
endif()

list(APPEND SOURCES ${PLATFORM_SOURCES})

add_library(gc STATIC ${SOURCES})
set_target_properties(gc PROPERTIES LINKER_LANGUAGE C)

add_executable(demo demo.c)
target_link_libraries(demo gc)