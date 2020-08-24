cmake_minimum_required(VERSION 2.4)
project(aruco_markers)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 OLD)
	cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)

set (CMAKE_CXX_STANDARD 11)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)

conan_basic_setup(TARGETS)

#find_package(OpenCV REQUIRED)
#include_directories(${OPENCV_INCLUDE_DIRS})
#include_directories(${PROJECT_SOURCE_DIR}/include)


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
