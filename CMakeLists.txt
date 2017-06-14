cmake_minimum_required (VERSION 2.6)

PROJECT(Bench)

find_package(Threads)
add_executable(Benchmark main.cpp)
target_link_libraries (Benchmark ${CMAKE_THREAD_LIBS_INIT})
