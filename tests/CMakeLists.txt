cmake_minimum_required(VERSION 3.14)
project(tests CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(CTest)
enable_testing()
include(GoogleTest)

function(test_case NAME)
  add_executable("${NAME}" "src/${NAME}.cpp")
  target_link_libraries("${NAME}" PRIVATE gtest_main gc)
  target_compile_options("${NAME}" PRIVATE -fprofile-arcs -ftest-coverage -g -O0)
  target_link_options("${NAME}" PRIVATE -fprofile-arcs -ftest-coverage)
  gtest_discover_tests("${NAME}" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endfunction()

test_case(hashmap_tests)
test_case(gc_tests)
