
set(camera_calibration_src
    src/main.cpp
   )
add_executable(camera_calibration ${camera_calibration_src})
target_link_libraries(camera_calibration
    PRIVATE CONAN_PKG::opencv
    )

target_compile_options(camera_calibration
    PRIVATE -O3 -std=c++11
    )


