cmake_minimum_required(VERSION 3.31)
add_executable(supernx)
target_sources(supernx PRIVATE
    main.c nx_sys.c nx_sys.h
    loader/types.h loader/ld.c loader/nsp.c loader/nsp.h loader/nsz.c loader/nsz.h
    horizon/hos.c horizon/hos_ld.c horizon/keys_db.c horizon/keys_fs.c horizon/keys_db.h horizon/hos.h horizon/tik.c horizon/tik.h horizon/content_fs.c
    fs_fmt/pfs.c fs_fmt/pfs.h fs_fmt/submission_package.c fs_fmt/submission_package.h fs_fmt/offset_file.c fs_fmt/offset_file.h fs_fmt/content_archive.c fs_fmt/content_archive.h fs_fmt/aes_file.c fs_fmt/aes_file.h fs_fmt/romfs.c fs_fmt/romfs.h

    hle_services/types.h hle_services/svt/title_ver.c hle_services/svt/title_ver.h
)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")

target_include_directories(supernx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>)
target_link_libraries(supernx PRIVATE core mbedtls)
if(zstd_FOUND)
    target_link_libraries(supernx PRIVATE zstd::libzstd_shared)
endif()
