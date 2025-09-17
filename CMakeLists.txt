cmake_minimum_required(VERSION 3.31)
project(funbox C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "-ffast-math -Wall -Werror -fcf-protection=full -march=x86-64-v3")
set(CMAKE_C_FLAGS_DEBUG "-Og -g")

#-fomit-frame-pointer (Enabled by default at -O1 and higher)
set(CMAKE_C_FLAGS_RELEASE "-D_FORTIFY_SOURCE=3 -O3 -DNDEBUG -flto")

option(FUNBOX_COMPILE_ALL_SUBSYSTEMS "Compile each available subsystem." ON)
option(FUNBOX_ENABLE_GUI "Enables the GUI (SDL3 for now)." ON)

find_package(zstd 1.4.0)
if(zstd_FOUND)
endif()
find_package(SDL3 REQUIRED)

add_subdirectory(libraries)
add_subdirectory(src)

