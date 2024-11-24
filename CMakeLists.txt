cmake_minimum_required(VERSION 3.15...3.31)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Structure LANGUAGES CXX)

include(CTest)
enable_testing()

add_subdirectory(src)