cmake_minimum_required(VERSION 3.31)
project(funbox C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "-ffast-math -Wall")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

option(FUNBOX_COMPILE_ALL_SUBSYSTEMS "Compile each available subsystem." ON)

add_subdirectory(libraries)
add_subdirectory(src)

