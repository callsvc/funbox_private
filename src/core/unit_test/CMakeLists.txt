cmake_minimum_required(VERSION 3.31)

add_executable(robin_test robin_test.c)
target_link_libraries(robin_test PRIVATE core)

add_executable(userfs_test userfs_test.c)
target_link_libraries(userfs_test PRIVATE core)