cmake_minimum_required(VERSION 3.22)
project(conv_gpu LANGUAGES CUDA CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(src)

add_executable(
    conv_gpu
    "src/conv/conv.cu"
    "src/pgm/pgm.cpp"
    "src/main.cpp"
)

set_target_properties(conv_gpu PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
set_target_properties(conv_gpu PROPERTIES CUDA_RESOLVE_DEVICE_SYMBOLS ON)
