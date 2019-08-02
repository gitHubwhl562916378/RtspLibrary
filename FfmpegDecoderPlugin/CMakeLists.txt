cmake_minimum_required(VERSION 3.0.0)
project(FfmpegDecoderPlugin VERSION 0.1.0)
add_compile_options(-std=c++17 -fpermissive)

include_directories(/usr/local/ffmpeg/include ..)
link_directories(/usr/local/ffmpeg/lib)

aux_source_directory(. SRCS)
add_library(${PROJECT_NAME} SHARED ${SRCS})
target_link_libraries(${PROJECT_NAME} avcodec avformat avutil swresample swscale pthread)
