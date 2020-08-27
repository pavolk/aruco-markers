cmake_minimum_required(VERSION 2.4)
project(aruco_markers)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 OLD)
	cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)

set (CMAKE_CXX_STANDARD 11)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)

conan_basic_setup(TARGETS)

add_subdirectory(camera_calibration)

add_subdirectory(create_markers)
add_subdirectory(detect_marker)
add_subdirectory(pose_estimation)
add_subdirectory(draw_cube)

