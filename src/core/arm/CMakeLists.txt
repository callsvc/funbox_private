add_library(dynrec STATIC)

target_sources(dynrec PUBLIC dynrec.c dynrec.h frontend/aarch64/arm64_2irs.c frontend/aarch64/arm64_table.c frontend/aarch64/arm64_fd.c frontend/aarch64/arm64_types.h details/aarch64.h ir/types.h th_context.c th_context.h jit.c jit.h types.h)

target_link_libraries(dynrec PUBLIC core)

add_subdirectory(unit_tests)