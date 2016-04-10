cmake_minimum_required(VERSION 2.8)
project(Sparrow)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
enable_testing()

aux_source_directory (./ "BASIC_SRC")
add_library(basic STATIC ${BASIC_SRC})

#测试暂时放下
#file(COPY ./test/testing_files DESTINATION test)
#add_subdirectory(./test)

add_subdirectory(./vm)
add_subdirectory(./pre_process)
add_subdirectory(./lib)
