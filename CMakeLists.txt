cmake_minimum_required (VERSION 2.8)

#### USER DEFINED ##############################################################

#### BASIC SETTINGS ############################################################

include_directories(.)

set (CMAKE_CXX_FLAGS "-std=c++17 -msse4.2 -Wall -Wextra -O3 -g -pthread")
find_package(Threads)

#### TARGETS ###################################################################

add_executable(seq_domi source/sequential_dominant.cpp)
add_executable(par_domi source/parallel_dominant.cpp)
add_executable(seq_suitor source/sequential_suitor.cpp)
add_executable(par_suitor source/parallel_suitor.cpp)
add_executable(par_free_suitor source/parallel_free_suitor.cpp)
add_executable(trans_file tools/trans_file.cpp)

## For debugging, prevents optimizing out
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-3")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gdwarf-3")


# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g -O2")
