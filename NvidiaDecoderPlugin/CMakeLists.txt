cmake_minimum_required(VERSION 3.0.0)
project(NvidiaDecoderPlugin VERSION 0.1.0)
add_compile_options(-std=c++17 -fpermissive)

include_directories(/usr/local/cuda/include)
include_directories(/usr/local/ffmpeg/include)
include_directories(Utils)
include_directories(..)
link_directories(/usr/local/ffmpeg/lib)

aux_source_directory(. SRCS)
add_library(${PROJECT_NAME} SHARED ${SRCS})
target_link_libraries(${PROJECT_NAME} nvcuvid cuda)
target_link_libraries(${PROJECT_NAME} pthread)
