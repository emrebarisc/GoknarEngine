cmake_minimum_required(VERSION 3.20)

project(IMGUI)

set(CMAKE_CXX_STANDARD 11)

set(LIBRARY_SOURCE)
file(GLOB sourceFiles "*.c" "*.cpp" "*.cxx")
list(APPEND LIBRARY_SOURCE ${sourceFiles})
add_library(IMGUI ${LIBRARY_SOURCE})

target_include_directories(IMGUI PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/)