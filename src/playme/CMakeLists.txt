cmake_minimum_required(VERSION 3.31)
add_executable(playme main.c vm/coffee.c vm/coffee.h vm/cpool.c vm/cpool.h vm/class.c vm/method.c vm/types.h vm/types.c vm/interpreter.c manifest.c manifest.h)

target_include_directories(playme PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>)

target_link_libraries(playme PUBLIC core)