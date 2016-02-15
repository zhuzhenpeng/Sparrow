add_subdirectory(./gtest-1.7.0)
include_directories(./gtest-1.7.0/include)

add_executable(eval_test eval_test.cc)
target_link_libraries(eval_test gtest gtest_main basic)
add_test(eval_test eval_test)
