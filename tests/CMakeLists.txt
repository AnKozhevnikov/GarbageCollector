cmake_minimum_required(VERSION 3.14)
project(tests CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(CTest)
enable_testing()
include(GoogleTest)

function(test_case NAME)
  add_executable("${NAME}" "src/${NAME}.cpp")
  target_link_libraries("${NAME}" PRIVATE gtest_main gc)
  gtest_discover_tests("${NAME}" WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endfunction()

test_case(hashmap_tests)
test_case(gc_tests)