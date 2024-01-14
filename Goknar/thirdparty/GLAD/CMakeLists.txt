cmake_minimum_required(VERSION 3.20)

project(GLAD)

set(CMAKE_CXX_STANDARD 11)

set(LIBRARY_SOURCE)
file(GLOB_RECURSE sourceFiles "GLAD/src/*.c" "GLAD/src/*.cpp" "GLAD/src/*.cxx")
list(APPEND LIBRARY_SOURCE ${sourceFiles})
add_library(GLAD ${LIBRARY_SOURCE})

target_include_directories(GLAD PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/GLAD/include)
