cmake_minimum_required(VERSION 3.15)
project(shell-starter-cpp)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE CORE_SOURCES src/string_utils.cpp src/Parser.cpp src/redirect.cpp src/reading.cpp)

add_executable(shell src/main.cpp ${CORE_SOURCES}) 
target_link_libraries(shell PRIVATE readline)


#GoogleTest setup 
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.16.0
)
# For Windows compatibility: prevents gtest from overriding compiler settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) 
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(run_tests tests/tests.cpp ${CORE_SOURCES})

target_link_libraries(run_tests PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(run_tests)
