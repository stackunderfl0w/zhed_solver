cmake_minimum_required(VERSION 3.25)
project(zhed C)

set(CMAKE_C_STANDARD 11)
#o2 seems to be faster than o3 or ofast
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -mtune=native")

add_executable(zhed main.c hashtable.h)
target_link_libraries(zhed m)
