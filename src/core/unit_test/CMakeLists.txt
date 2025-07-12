cmake_minimum_required(VERSION 3.31)

add_executable(test_robin test_robin.c)
target_link_libraries(test_robin PRIVATE core)
