
set(pose_estimation_src
    src/main.cpp
   )
add_executable(pose_estimation ${pose_estimation_src})
target_link_libraries(pose_estimation
    PRIVATE CONAN_PKG::opencv
    )

target_compile_options(pose_estimation
    PRIVATE -O3 -std=c++11
    )


