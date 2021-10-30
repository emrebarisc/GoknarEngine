cmake_minimum_required(VERSION 3.20)

project(TINYXML)

set(CMAKE_CXX_STANDARD 11)

set(LIBRARY_SOURCE)
file(GLOB_RECURSE sourceFiles "src/*.c" "src/*.cpp" "src/*.cxx")
file(GLOB_RECURSE headerFiles "include/*.h" "include/*.hpp" "include/*.hxx")

list(APPEND LIBRARY_SOURCE ${sourceFiles} ${headerFiles})
add_library(TINYXML ${LIBRARY_SOURCE})

target_include_directories(TINYXML PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include)
