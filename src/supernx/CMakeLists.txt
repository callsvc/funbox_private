cmake_minimum_required(VERSION 3.31)
add_executable(supernx main.c nx_sys.c nx_sys.h loader/types.h loader/loader.c loader/nsp.c loader/nsp.h horizon/hos.c horizon/keys_db.c horizon/keys_db.h horizon/hos.h horizon/tik.c horizon/tik.h fs_fmt/pfs.c fs_fmt/pfs.h fs_fmt/submission_package.c fs_fmt/submission_package.h fs_fmt/offset_file.c fs_fmt/offset_file.h fs_fmt/content_archive.c fs_fmt/content_archive.h fs_fmt/aes_file.c fs_fmt/aes_file.h)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")

target_include_directories(supernx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>)
target_link_libraries(supernx PRIVATE core mbedtls)