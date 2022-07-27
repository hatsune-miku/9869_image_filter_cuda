cmake_minimum_required(VERSION 3.22)
project(image_filter_9869 LANGUAGES CUDA CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)
file(
    GLOB_RECURSE sources
    "src/*.cpp"
    "src/pgm/*.cpp"
)

add_executable(
    image_filter_9869
    ${sources}
)
