cmake_minimum_required(VERSION 3.31)

add_library(core STATIC envos.c error.c procinfo.c strings.c util.c types.h algo/list.c algo/list.h algo/vector.c algo/vector.h algo/set.c algo/set.h algo/ht.c algo/ht.h fs/fs.c fs/dir.c fs/dir.h fs/dirrec.c fs/types.h fs/file.c fs/file.h fs/mapfile.c fs/mapfile.h zip_fs/zip.h zip_fs/zip_dir.c zip_fs/zip_file.c)
target_link_libraries(core PUBLIC m libzip::zip)

target_include_directories(core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>)

add_subdirectory(arm)
