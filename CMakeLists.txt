cmake_minimum_required(VERSION 3.15)

project(Mishelle)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

add_executable(mishelle ${SOURCE_FILES})

target_link_libraries(shell PRIVATE readline)

