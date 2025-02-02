cmake_minimum_required(VERSION 3.14.0)
project(garbage_collcetor VERSION 0.1.0 LANGUAGES C)

include_directories(include src)

add_subdirectory(tests)

file(GLOB_RECURSE SOURCES "src/*.c")

add_library(gc ${SOURCES})
set_target_properties(gc PROPERTIES LINKER_LANGUAGE C)

add_executable(demo demo.c)
target_link_libraries(demo gc)