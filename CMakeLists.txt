cmake_minimum_required(VERSION 3.14.0)
project(garbage_collcetor VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include_directories(include src)

add_subdirectory(tests)

file(GLOB_RECURSE SOURCES "src/*.c")

if(APPLE)
    file(GLOB_RECURSE PLATFORM_SOURCES "platform/apple/*.c")
elseif(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        file(GLOB_RECURSE PLATFORM_SOURCES "platform/linux/*.c")
    endif()
endif()

list(APPEND SOURCES ${PLATFORM_SOURCES})

add_library(gc STATIC ${SOURCES})
target_compile_options(gc PRIVATE -std=c11 -D_GNU_SOURCE)

add_executable(demo demo.c)
target_link_libraries(demo gc)

