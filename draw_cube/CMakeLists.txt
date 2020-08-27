
set(draw_cube_src
    src/main.cpp
   )
add_executable(draw_cube ${draw_cube_src})
target_link_libraries(draw_cube
    PRIVATE CONAN_PKG::opencv
    )

target_compile_options(draw_cube
    PRIVATE -O3 -std=c++11
    )


