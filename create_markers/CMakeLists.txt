
set(generate_marker_src
    src/create_marker.cpp
   )
add_executable(generate_marker ${generate_marker_src})
target_link_libraries(generate_marker 
    PRIVATE CONAN_PKG::opencv
    )

target_compile_options(generate_marker
    PRIVATE -O3 -std=c++11
    )


set(generate_board_src
    src/create_board.cpp
   )
add_executable(generate_board ${generate_board_src})
target_link_libraries(generate_board 
    PRIVATE CONAN_PKG::opencv
    )

target_compile_options(generate_board
    PRIVATE -O3 -std=c++11
    )
