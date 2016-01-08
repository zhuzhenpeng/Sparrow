cmake_minimum_required(VERSION 2.8)
project(Sparrow)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

aux_source_directory (./ "SRC")

add_executable (main ${SRC})
