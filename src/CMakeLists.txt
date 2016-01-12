cmake_minimum_required(VERSION 2.8)
project(Sparrow)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
enable_testing()

aux_source_directory (./ "SRC")

add_library(base STATIC ${SRC})

file(COPY ./test/testing_files DESTINATION test)
add_subdirectory(./test)
