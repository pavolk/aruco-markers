
set(detect_markers_src
    src/main.cpp
   )
add_executable(detect_markers ${detect_markers_src})
target_link_libraries(detect_markers
    PRIVATE CONAN_PKG::opencv
    )

target_compile_options(detect_markers
    PRIVATE -O3 -std=c++11
    )


