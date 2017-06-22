cmake_minimum_required (VERSION 2.6)

PROJECT(Bench)
SET(CMAKE_CXX_FLAGS "-std=c++14 -O3")
find_package(Threads)
add_executable(Benchmark main.cpp)
target_link_libraries (Benchmark ${CMAKE_THREAD_LIBS_INIT})

add_executable(Bitvec yolo.cpp)